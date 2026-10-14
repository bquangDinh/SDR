#include "rtl_audio_thread.h"

#include <QDebug>
#include <chrono>

#include "core/demodulators/amdemod.h"
#include "core/demodulators/nfmdemod.h"
#include "core/demodulators/wfmdemod.h"
#include "core/demodulators/cwdemod.h"
#include "core/demodulators/dsbdemod.h"
#include "core/demodulators/lsbdemod.h"
#include "core/demodulators/usbdemod.h"
#include "core/demodulators/rawdemod.h"

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

void RtlAudioThread::setCWShiftAmount(int cwShiftAmount) {
    if (_demod) {
        _demod->set_cw_shift(cwShiftAmount);
    }
}

void RtlAudioThread::setSquelchAmount(uint32_t squelch) {
    if (_demod) {
        _demod->set_squelch_amount(squelch);
    }
}

void RtlAudioThread::setSquelch(bool squelch) {
    if (_demod) {
        _demod->set_squelch(squelch);
    }
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
    if (_demod) _demod->set_bandwidth(bw);
}

void RtlAudioThread::setDemodMode(int mode) {
    if (mode == Constants::DemodulateMode::NFM) {
        _requestMode = DemodMode::NFM;
    } else if (mode == Constants::DemodulateMode::AM) {
        _requestMode = DemodMode::AM;
    } else if (mode == Constants::DemodulateMode::LSB) {
        _requestMode = DemodMode::LSB;
    } else if (mode == Constants::DemodulateMode::USB) {
        _requestMode = DemodMode::USB;
    } else if (mode == Constants::DemodulateMode::WFM) {
        _requestMode = DemodMode::WFM;
    } else if (mode == Constants::DemodulateMode::DSB) {
        _requestMode = DemodMode::DSB;
    } else if (mode == Constants::DemodulateMode::CW) {
        _requestMode = DemodMode::CW;
    } else if (mode == Constants::DemodulateMode::RAW) {
        _requestMode = DemodMode::RAW;
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

            _demod->set_output_sample_rate(AUDIO_SAMPLE_RATE);

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
            return std::make_unique<AmDemod>();
        case DemodMode::NFM:
            return std::make_unique<NfmDemod>();
        case DemodMode::WFM:
            return std::make_unique<WfmDemod>();
        case DemodMode::LSB:
            return std::make_unique<LsbDemod>();
        case DemodMode::USB:
            return std::make_unique<UsbDemod>();
        case DemodMode::DSB:
            return std::make_unique<DsbDemod>();
        case DemodMode::CW:
            return std::make_unique<CwDemod>();
        case DemodMode::RAW:
            return std::make_unique<RawDemod>();
        case DemodMode::NONE:
            return nullptr;
    }

    return nullptr;
}