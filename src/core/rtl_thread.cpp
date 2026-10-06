#include "core/rtl_thread.h"

#include <vector>
#include <cmath>
#include <assert.h>

std::vector<float> RtlThread::make_filter(uint32_t fPass, uint32_t fStop, uint32_t fs) {
    float fPassNormalized = static_cast<float>(fPass) / static_cast<float>(fs);
    float fStopNormalized = static_cast<float>(fStop) / static_cast<float>(fs);

    float fc = (fStopNormalized + fPassNormalized) / 2.0f;

    float BW = fStopNormalized - fPassNormalized;

    int M = ceil(4.0f / BW);

    if (M % 2 != 0) {
        // Not even
        M += 1;
    }

    std::vector<float> kernel(M + 1);

    float wi = 0;
    float sum = 0;

    for (int i = 0; i < kernel.size(); ++i) {
        if (i == M / 2) {
            kernel[i] = 2 * M_PI * fc;
        } else {
            wi =  0.42f - 0.5f * cos(2 * M_PI * i / static_cast<float>(M)) + 0.08f * cos(4 * M_PI * i / static_cast<double>(M));

            kernel[i] = wi * sin(2 * M_PI * fc * (i - (static_cast<float>(M) / 2.0f))) / (i - (static_cast<float>(M) / 2.0f));
        }

        sum += kernel[i];
    }

    // Normalize so every value add up to one in kernel
    for (int i = 0; i < kernel.size(); ++i) {
        kernel[i] /= sum;
    }

    return kernel;
}

std::vector<std::complex<float>> RtlThread::filter(std::vector<std::complex<float>>& iqStream, std::vector<float>& taps, int decimate) {
    std::vector<std::complex<float>> result;

    std::complex<float> y = 0;

    int N = iqStream.size();
    int M = taps.size();

    for (int n = 0; n < N; n += decimate) {
        y = 0;

        for (int k = 0; k < M; ++k) {
            if (n - k < 0) {
                if (_filterHistory.size() > 0) {
                    y += taps[k] * _filterHistory[_filterHistory.size() + (n - k)];
                }
            } else {
                y += taps[k] * iqStream[n - k];
            }
        }

        result.push_back(y);
    }

    _filterHistory.clear();

    _filterHistory.reserve(M - 1);

    // Save the last taps.size() - 1 samples into history
    for (int i = 0; i < M - 1; ++i) {
        _filterHistory.push_back(iqStream[N - M - 1 - i]);
    }

    return result;
}

void RtlThread::demodulate(std::vector<std::complex<float>>& iqStream) {
    std::vector<float> audio;

    if (iqStream.empty()) return;

    audio.reserve(iqStream.size());

    if (_hasLastSample) {
        std::complex<float> d0 = iqStream[0] * std::conj(_lastDecimatedSample);
        audio.push_back(std::arg(d0));
    }

    // handle the rest of this block normally
    for (size_t n = 1; n < iqStream.size(); ++n) {
        std::complex<float> d = iqStream[n] * std::conj(iqStream[n - 1]);
        audio.push_back(std::arg(d));
    }

    // save state for the NEXT call
    _lastDecimatedSample = iqStream.back();
    _hasLastSample = true;

    if (audio.empty()) return;

    const float maxDeviationHz = 5000.0f;
    const float scale = (48000) / (2.0f * M_PI * maxDeviationHz);

    for (auto &sample : audio) {
        sample *= scale;
    }

    std::vector<int16_t> pcm(audio.size());

    for (size_t i = 0; i < audio.size(); ++i) {
        float clamped = std::clamp(audio[i], -1.0f, 1.0f);
        pcm[i] = static_cast<int16_t>(clamped * 32767.0f);
    }

    _audioOutputBuffer.insert(_audioOutputBuffer.end(), pcm.begin(), pcm.end());

    const size_t flushThreshold = 960;
    if (_audioOutputBuffer.size() >= flushThreshold) {
        qint64 bytesToWrite = _audioOutputBuffer.size() * sizeof(int16_t);
        qint64 bytesWritten = audioDevice->write(reinterpret_cast<const char*>(_audioOutputBuffer.data()), bytesToWrite);

        if (bytesWritten < bytesToWrite) {
            qDebug() << "AUDIO UNDERRUN: wanted to write" << bytesToWrite << "but only wrote" << bytesWritten;
            // keep the unwritten remainder for next time, instead of dropping it:
            size_t samplesWritten = bytesWritten / sizeof(int16_t);
            _audioOutputBuffer.erase(_audioOutputBuffer.begin(), _audioOutputBuffer.begin() + samplesWritten);
        } else {
            _audioOutputBuffer.clear();
        }
    }
}

void RtlThread::get_iq_stream(const char* buf, int n, std::vector<std::complex<float>>& out) {
    assert(n % 2 == 0 && "the BUFFER SIZE must be dividable by two");

    assert(out.size() == (n / 2) && "The vector size is not allocated");

    const auto *u = reinterpret_cast<const unsigned char *>(buf); // 0 - 255

    for (int i = 0; i < n / 2; ++i)
        out[i] = { (u[2 * i]     - 127.5f) / 127.5f,
                (u[2 * i + 1] - 127.5f) / 127.5f }; // -1.0f to 1.0f
}

void RtlThread::fft(std::vector<std::complex<float>>& x, int N, std::complex<float>* outBuf) {
    if (N <= 1) {
        // Since there is only one element so outBuf only store one element
        outBuf[0] = x[0];

        return;
    }

    std::vector<std::complex<float>> even_terms(N / 2);
    std::vector<std::complex<float>> odd_terms(N / 2);

    for (int i = 0; i < N / 2; ++i) {
        even_terms[i] = x[2 * i];
        odd_terms[i] = x[2 * i + 1];
    }

    std::complex<float>* E = (std::complex<float>*)std::malloc((N / 2) * sizeof(std::complex<float>));
    std::complex<float>* O = (std::complex<float>*)std::malloc((N / 2) * sizeof(std::complex<float>));

    fft(even_terms, N / 2, E);
    fft(odd_terms, N / 2, O);

    for (int k = 0; k < N / 2; ++k) {
        std::complex<float> W = std::polar(1.0f, -2.0f * float(M_PI) * k / N);
        outBuf[k] = E[k] + W * O[k];
        outBuf[k + N / 2] = E[k] - W * O[k];
    }

    free(E);

    free(O);
}

void RtlThread::sample_fft(std::vector<std::complex<float>>& iqStream, int binsCount, std::complex<float>* outBuf) {
    return fft(iqStream, binsCount, outBuf);
}

void RtlThread::sample(std::vector<std::complex<float>>& iqStream, int binsCount, std::complex<float> *outBuf) {
    // compute e_term once
    std::vector<std::complex<float>> tw(binsCount);
    for (int m = 0; m < binsCount; ++m)
        tw[m] = std::polar(1.0f, -2.0f * float(M_PI) * m / binsCount);

    for (int k = 0; k < binsCount; ++k) {
        std::complex<float> sum = 0;
        for (int i = 0; i < binsCount; ++i)
            sum += iqStream[i] * tw[(long long)k * i % binsCount];   // angle wraps every full turn
        outBuf[k] = sum;
    }
}

void RtlThread::toDBFS(const std::complex<float>* X, int binsCount, std::vector<float> &out) {
    for (int k = 0; k < binsCount; ++k) {
        int src = (k + binsCount / 2) % binsCount;

        float mag = std::abs(X[src]) / binsCount; // normalized

        out[k] = 20.0f * std::log10(mag + 1e-20f); // avoid log(10)(0)
    }
}
