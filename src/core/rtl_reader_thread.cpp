#include "rtl_reader_thread.h"

#include "core/sdr_device_manager.h"
#include <QDebug>
#include <thread>

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

    if (currentDevice == nullptr) {
        qDebug() << "No device selected";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return;
    }

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

        if (_correctIQ) {
            _muI += _alpha * (I - _muI);
            _muQ += _alpha * (Q - _muQ);

            I -= _muI;
            Q -= _muQ;
        }

        if (_invertSpectrum) {
            std::swap(I, Q);
        }

        (*chunk)[i] = {I, Q};
    }

    Chunk c = chunk;

    _queue->push(c);

    _latestChunk->put(c);
}