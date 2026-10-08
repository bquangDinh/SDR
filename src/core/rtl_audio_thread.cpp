#include "rtl_audio_thread.h"

#include <QDebug>
#include <complex>

#include "core/sdr_device_manager.h"

void RtlAudioThread::pause() {
    std::lock_guard<std::mutex> lk(_m);

    _paused = true;
}

void RtlAudioThread::stop() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _stop = true;
    }

    _cv.notify_all();
}

void RtlAudioThread::resume() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _paused = false;
    }

    _cv.notify_all();
}

void RtlAudioThread::setOffsetFreq(double offset) {
    this->_offsetFreq = offset;

    _rotStep = std::polar(1.0, -2.0 * M_PI * offset / _sampleRate);
}

void RtlAudioThread::run() {
    // Setup --- only once
    QAudioFormat format;

    format.setSampleRate(48000); // 48 kHz
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format);

    audioSink->setBufferSize(9600);

    connect(audioSink, &QAudioSink::stateChanged, [](QAudio::State state) {
        qDebug() << "Audio sink state changed to:" << state;
    });

    audioDevice = audioSink->start();

    while (true) {
        {
            std::unique_lock<std::mutex> lk(_m);

            _cv.wait(lk, [&] {
                return !_paused || _stop;
            });

            if (_stop) break;
        }

        if (_init == false) {
            if (init()) _init = true;
        }

        if (_init == true) {
            read();
        }
    }

    // Cleanup
    audioSink->stop();

    audioSink->deleteLater();

    audioSink = nullptr;

    audioDevice = nullptr;

    qDebug() << "Destroyed audio device";
}

bool RtlAudioThread::init() {
    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice == nullptr) {
        qDebug() << "No device selected";

        return false;
    }

    std::vector<float> taps = make_filter(_bandwidth, 24000, currentDevice->config().sampleRate);

    _dec = new FastDecimator(taps, 50);

    return true;
}

std::vector<float> RtlAudioThread::make_filter(const uint32_t fPass, const uint32_t fStop, const uint32_t sampleRate) {
    float fPassNormalized = static_cast<float>(fPass) / static_cast<float>(sampleRate);
    float fStopNormalized = static_cast<float>(fStop) / static_cast<float>(sampleRate);

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

void RtlAudioThread::demodulate() {
    std::vector<float> audio;

    if (decimated.empty()) return;

    audio.reserve(decimated.size());

    if (_hasLastSample) {
        std::complex<float> d0 = decimated[0] * std::conj(_lastDecimatedSample);
        audio.push_back(std::arg(d0));
    }

    // handle the rest of this block normally
    for (size_t n = 1; n < decimated.size(); ++n) {
        std::complex<float> d = decimated[n] * std::conj(decimated[n - 1]);
        audio.push_back(std::arg(d));
    }

    // save state for the NEXT call
    _lastDecimatedSample = decimated.back();
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

void RtlAudioThread::demodulate_am() {
    std::vector<float> audio;

    if (decimated.empty()) return;

    audio.reserve(decimated.size());

    if (_hasLastSample) {
        float mag = std::abs(_lastDecimatedSample);

        audio.push_back(mag);
    }

    // handle the rest of this block normally
    for (size_t n = 0; n < decimated.size(); ++n) {
        audio.push_back(std::abs(decimated[n]));
    }

    // save state for the NEXT call
    _lastDecimatedSample = decimated.back();
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

void RtlAudioThread::shift(const std::complex<float>* data, size_t size, std::vector<std::complex<float>>& out) {
    out.resize(size);

    for (size_t i = 0; i < size; ++i) {
        out[i] = data[i] * std::complex<float>(_rot);

        _rot *= _rotStep;
    }

    _rot /= std::abs(_rot); // normalize in case the value drift due to rounding off
}

void RtlAudioThread::read() {
    if (Chunk c = _q->pop(std::chrono::milliseconds(100))) {
        decimated.clear();

        if (_offsetFreq != 0) {
            shift(c->data(), c->size(), shifted);

            _dec->process(shifted.data(), shifted.size(), decimated);
        } else {
            _dec->process(c->data(), c->size(), decimated);
        }

        _decimatedQueue->push(std::make_shared<const std::vector<std::complex<float>>>(decimated));

        if (_demod == Constants::DemodulateMode::AM) {
            demodulate_am();
        } else if (_demod == Constants::DemodulateMode::NFM) {
            demodulate();
        } else {
            qDebug() << "No supported mode";
        }
    }
}