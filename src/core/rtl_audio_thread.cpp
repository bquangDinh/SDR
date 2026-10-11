#include "rtl_audio_thread.h"

#include <QDebug>

#include "core/demodulators/amdemod.h"
#include "core/demodulators/nfmdemod.h"

void RtlAudioThread::pause() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _paused = true;
    }

    if (_demod) _demod->reset();

    _mixer.reset();

    _audioOutput.reset();
}

void RtlAudioThread::stop() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _stop = true;
    }

    if (_demod) _demod->reset();

    _mixer.reset();

    _audioOutput.reset();

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
    _mixer.setOffsetFreq(offset);
}

void RtlAudioThread::setSampleRate(uint32_t sampleRate) {
    if (_sampleRate == sampleRate) return;

    _sampleRate = sampleRate;

    _mixer.setSampleRate(sampleRate);

    if (_demod) {
        _demod->set_input_sample_rate(sampleRate);
    }
}

void RtlAudioThread::setBandwidth(uint32_t bw) {
    //
}

void RtlAudioThread::setDemodMode(int mode) {
    if (mode == Constants::DemodulateMode::NFM) {
        _requestMode = DemodMode::NFM;
    } else if (mode == Constants::DemodulateMode::AM) {
        _requestMode = DemodMode::AM;
    } else {
        _requestMode = DemodMode::NFM;
    }
}

void RtlAudioThread::run() {
    _audioOutput.init();

    _audioOutput.start();

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

    _audioOutput.stop();
}

void RtlAudioThread::read() {
    if (Chunk c = _q->pop(std::chrono::milliseconds(100))) {
        const DemodMode requestedDemodMode = _requestMode.load();

        if (requestedDemodMode != _activeMode) {
            _demod = make_demodulator(requestedDemodMode);

            _demod->set_input_sample_rate(_sampleRate);

            _demod->set_output_sample_rate(48e3);

            _demod->connect_decimated_callback([this](const std::complex<float>* data, size_t n) {
                if (_decimatedQueue) _decimatedQueue->push(std::make_shared<const std::vector<std::complex<float>>>(data, data + n));
            });

            _activeMode = requestedDemodMode;
        }

        _mixer.process(c->data(), c->size(), _shifted);

        _demod->process(_shifted.data(), _shifted.size(), _audio);

        _audioOutput.write(_audio);
    }
}

std::unique_ptr<Demodulator> RtlAudioThread::make_demodulator(DemodMode m) {
    switch (m) {
        case DemodMode::AM:
            return std::make_unique<AMDemod>();
        case DemodMode::NFM:
            return std::make_unique<NfmDemod>();
        case DemodMode::NONE:
            return nullptr;
    }

    return nullptr;
}