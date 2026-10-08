#include "rtl_fft_thread.h"

#include <QDebug>
#include <chrono>
#include <thread>

#include <core/spectrum.h>

void RtlFFTThread::pause() {
    std::lock_guard<std::mutex> lk(_m);

    _paused = true;
}

void RtlFFTThread::stop() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _stop = true;
    }

    _cv.notify_all();
}

void RtlFFTThread::resume() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _paused = false;
    }

    _cv.notify_all();
}

void RtlFFTThread::run() {
    // Setup
    auto next = std::chrono::steady_clock::now();

    Spectrum fullSpectrum(FFT_BINS);
    Spectrum narrowSpectrum(NARROW_FFT_BINS);

    std::vector<std::complex<float>> decChain;

    const size_t NARROW_MAX_KEEP = 4 * NARROW_FFT_BINS;

    // Telemetry stuff
    uint32_t totalFullSpectrumTime = 0;
    uint32_t totalNarrowSpectrumTime = 0;
    uint32_t fullSpectrumCount = 0, narrowSpectrumCount = 0;
    uint32_t fullSpectrumDataCount = 0;
    uint32_t narrowSpectrumDataCount = 0;

    // Loop through every frame at 30 FPS
    while (true) {
        {
            std::unique_lock<std::mutex> lk(_m);

            _cv.wait(lk, [&] {
                return !_paused || _stop;
            });

            if (_stop) break;
        }

        if (Chunk c = _c->take()) {
            auto t0 = std::chrono::steady_clock::now();

            fullSpectrum.compute(c->data(), c->size(), fullSpectrumOutput);

            auto t1 = std::chrono::steady_clock::now();

            totalFullSpectrumTime += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

            fullSpectrumDataCount += c->size();

            fullSpectrumCount++;

            emit spectrumReady(QList<double>(fullSpectrumOutput.begin(), fullSpectrumOutput.end()));
        }

        while (Chunk c = _decimatedQueue->tryPop()) {
            decChain.insert(decChain.end(), c->begin(), c->end());
        }

        const size_t N   = NARROW_FFT_BINS;
        const size_t HOP = N / 4;

        size_t pos = 0;

        while (decChain.size() - pos >= N) {
            auto t0 = std::chrono::steady_clock::now();

            narrowSpectrum.compute(decChain.data() + pos, N, narrowSpectrumOutput);

            auto t1 = std::chrono::steady_clock::now();

            totalNarrowSpectrumTime += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

            narrowSpectrumDataCount += N;

            narrowSpectrumCount++;

            emit narrowSpectrumReady(QList<double>(narrowSpectrumOutput.begin(), narrowSpectrumOutput.end()));

            pos += HOP;
        }

        decChain.erase(decChain.begin(), decChain.begin() + pos);   // keep the unused tail

        next += std::chrono::milliseconds(33);

        std::this_thread::sleep_until(next);
    }

    double fullSpectrumAvg = static_cast<double>(totalFullSpectrumTime) / fullSpectrumCount;
    double narrowSpectrumAvg = static_cast<double>(totalNarrowSpectrumTime) / narrowSpectrumCount;
    double fullSpectrumDataAvg = static_cast<double>(fullSpectrumDataCount) / fullSpectrumCount;
    double narrowSpectrumDataAvg = static_cast<double>(narrowSpectrumDataCount) / narrowSpectrumCount;

    qDebug() << "Computing full spectrum of " << fullSpectrumDataAvg << " elements took " << fullSpectrumAvg << " us on average";
    qDebug() << "Computing narrow spectrum of " << narrowSpectrumDataAvg << " elements took " << narrowSpectrumAvg << " us on average";
}