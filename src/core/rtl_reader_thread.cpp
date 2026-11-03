#include "rtl_reader_thread.h"

#include "core/sdr_settings_manager.h"
#include "core/sdr_device_manager.h"
#include <QDebug>
#include <thread>
#include "core/telemetry.h"

RtlReaderThread::RtlReaderThread(ChunkQueue* q, LatestChunk* c, QObject* parent) : QThread(parent), _queue(q), _latestChunk(c) {
    _sampleRateSubId = SdrSettingsManager::instance().on_sample_rate_changed().subscribe([this](const uint32_t& v) { this->on_sample_rate_changed(v); });
}

RtlReaderThread::~RtlReaderThread() {
    SdrSettingsManager::instance().on_sample_rate_changed().unsubscribe(_sampleRateSubId);
}

void RtlReaderThread::pause() {
    std::lock_guard<std::mutex> lk(_m);

    _paused = true;
}

void RtlReaderThread::stop() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _stop = true;
    }

    _cv.notify_all();
}

void RtlReaderThread::resume() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _paused = false;
    }

    _cv.notify_all();
}

void RtlReaderThread::run() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(_m);

            _cv.wait(lk, [&] {
                return !_paused || _stop;
            });

            if (_stop) break;
        }

        read();
    }
}

void RtlReaderThread::read() {
    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();
    
    SdrSettingsManager& settingsManager = SdrSettingsManager::instance();

    if (currentDevice == nullptr) {
        qDebug() << "No device selected";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return;
    }

    Telemetry::instance().start_recording_readerThreadProcessingTime();

    ssize_t read = currentDevice->sdr_read_data_sync((void*)buf, BUFFER_SIZE);

    if (read < 0) {
        qDebug() << "read failed";

        return;
    }

    if (read == 0) return;

    const int n = static_cast<size_t>(read) / 2;

    // Make new chunk
    auto chunk = std::make_shared<std::vector<std::complex<float>>>(n);

    float I, Q;

    for (size_t i = 0; i < n; ++i) {
        // Skip boundary check here, could be bug if not careful
        I = (buf[2 * i]     - 127.5f) / 127.5f;
        Q = (buf[2 * i + 1] - 127.5f) / 127.5f;

        if (settingsManager.correctIQ()) {
            _muI += _alpha * (I - _muI);
            _muQ += _alpha * (Q - _muQ);

            I -= _muI;
            Q -= _muQ;
        }

        if (settingsManager.invertSpectrum()) {
            std::swap(I, Q);
        }

        (*chunk)[i] = {I, Q};
    }

    Chunk c = chunk;

    _queue->push(c);

    _latestChunk->put(c);

    Telemetry::instance().stop_recording_readerThreadProcessingTime();
}

void RtlReaderThread::on_sample_rate_changed(const uint32_t& v) {
    _alpha = 2 * M_PI * _correctIQCutOff / v;
}