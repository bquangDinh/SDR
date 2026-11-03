#include "amdemod.h"

#include <QDebug>
#include "core/telemetry.h"

void AmDemod::process(const std::complex<float>* in, size_t n, std::vector<float>& out) {
    if (_dirty) {
        if (init()) _dirty = false;
        else {
            qDebug() << "Demod: failed to init";

            return;
        }
    }

    Telemetry::instance().start_recording_AMdemodProcessingTime();

    _o1.clear();

    _dec1->process(in, n, _o1);

    _o2.clear();

    _dec2->process(_o1.data(), _o1.size(), _o2);

    if (!_squelch || (_squelch && !_squelcher.shouldSquelch(_o2.data(), _o2.size()))) {
        _am->process(_o2.data(), _o2.size(), out);

        // Perform a IIR single pole lowpass to remove carrier
        for (auto& sample : out) {
            _carrier = a0 * sample + b1 * _carrier;
            sample -= _carrier;
            sample /= (_carrier + 1e-9f);
        }
    } else {
        out.clear();
    }

    process_narrow_fft(_o2.data(), _o2.size(), PASSBAND);

    Telemetry::instance().stop_recording_AMdemodProcessingTime();
}

void AmDemod::reset() {
    // Force reset init
    _dirty = true;

    if (_dec1) _dec1->reset();
    if (_dec2) _dec2->reset();
    if (_am) _am->reset();
}

bool AmDemod::init() {
    if (_inSampleRate == 0 || _outSampleRate == 0) {
        qDebug() << "Demod: You forgot to set input sample rate or output sample rate";

        return false;
    }

    if (_inSampleRate < _outSampleRate) {
        qDebug() << "Demod: Input sample rate cannot be lower than output sample rate";

        return false;
    }

    if (_inSampleRate % _outSampleRate != 0) {
        qDebug() << "Demod: Input sample rate must be divisible into output sample rate";

        return false;
    }

    uint32_t midrate = 240e3;

    size_t d1 = _inSampleRate / midrate;
    size_t d2 = midrate / _outSampleRate;

    uint32_t passband = std::abs(_bandwidth) > 0 ? _bandwidth / 2 : PASSBAND;
    uint32_t stopband = passband + passband * 0.2; // 20% more

    std::vector<float> taps1 = Signal::Utils::make_lowpass(_inSampleRate, passband, midrate - stopband);
    std::vector<float> taps2 = Signal::Utils::make_lowpass(midrate, passband, stopband);

    _dec1 = std::make_unique<FastDecimator>(taps1, d1);
    _dec2 = std::make_unique<FastDecimator>(taps2, d2);
    _am = std::make_unique<AmDiscriminator>();

    b1 = std::exp(-2.0f * float(M_PI) * _fc / _outSampleRate);
    a0 = 1.0f - b1;

    return true;
}