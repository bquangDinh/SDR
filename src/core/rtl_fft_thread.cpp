#include "rtl_fft_thread.h"

#include <QDebug>
#include <chrono>
#include <thread>

#include "core/spectrum.h"
#include "core/sdr_settings_manager.h"
#include "core/telemetry.h"

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
    SdrSettingsManager& settingsManager = SdrSettingsManager::instance();

    // Setup
    auto next = std::chrono::steady_clock::now();

    Spectrum fullSpectrum(FFT_BINS);
    Spectrum narrowSpectrum(NARROW_FFT_BINS);

    std::vector<std::complex<float>> decChain;

    const size_t NARROW_MAX_KEEP = 4 * NARROW_FFT_BINS;

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
            Telemetry::instance().start_recording_FFTWideSpectrumProcessingTime();

            fullSpectrum.compute(c->data(), c->size(), fullSpectrumOutput);

            Telemetry::instance().stop_recording_FFTWideSpectrumProcessingTime();

            emit spectrumReady(QList<double>(fullSpectrumOutput.begin(), fullSpectrumOutput.end()));
        }

        if (Chunk c= _decimatedQueue->try_pop()) {
            decChain.insert(decChain.end(), c->begin(), c->end());

            double avgPower = compute_spectrum_power(c->data(), c->size());

            emit power(avgPower);
        }

        while (Chunk c = _decimatedQueue->try_pop()) {
            decChain.insert(decChain.end(), c->begin(), c->end());
        }
        
        const uint32_t bandwidth = settingsManager.bandwidth();
        const double NARROW_HOP_TIME = (bandwidth == 0 || bandwidth >= 48e3) ? 48e3 / 30 : bandwidth / 30;
        const size_t N   = NARROW_FFT_BINS;
        const size_t HOP = std::clamp<size_t>(NARROW_HOP_TIME, size_t(1), N);

        size_t pos = 0;

        while (decChain.size() - pos >= N) {
            Telemetry::instance().start_recording_FFTNarrowSpectrumProcessingTime();

            narrowSpectrum.compute(decChain.data() + pos, N, narrowSpectrumOutput);

            Telemetry::instance().stop_recording_FFTNarrowSpectrumProcessingTime();

            emit narrowSpectrumReady(QList<double>(narrowSpectrumOutput.begin(), narrowSpectrumOutput.end()));

            pos += HOP;
        }

        decChain.erase(decChain.begin(), decChain.begin() + pos);   // keep the unused tail

        next += std::chrono::milliseconds(33);

        std::this_thread::sleep_until(next);
    }
}

double RtlFFTThread::compute_spectrum_power(const std::complex<float>* data, size_t n) {
    double avg = 0;

    for (size_t i = 0; i < n; ++i) avg += std::norm(data[i]) / n;

    avg = 10.0 * std::log10(avg + 1e-17);

    return avg;
}