#include "amdemod.h"

#include <QDebug>

void AMDemod::process(const std::complex<float>* in, size_t n, std::vector<float>& out) {
    if (_dirty) {
        if (init()) _dirty = false;
        else {
            qDebug() << "Demod: failed to init";

            return;
        }
    }

    _iq.clear();

    _dec->process(in, n, _iq);
    _am->process(_iq.data(), _iq.size(), out);

    // Perform a IIR single pole lowpass to remove carrier
    for (auto& sample : out) {
        _carrier = a0 * sample + b1 * _carrier;
        sample -= _carrier;
        sample /= (_carrier + 1e-9f);
    }

    // Callback decimated
    if (_cb) _cb(_iq.data(), _iq.size());
}

void AMDemod::reset() {
    // Force reset init
    _dirty = true;

    if (_dec) _dec->reset();
    if (_am) _am->reset();
}

bool AMDemod::init() {
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

    size_t decimatedStep = _inSampleRate / _outSampleRate;

    std::vector<float> taps = Signal::Utils::make_lowpass(_inSampleRate, PASSBAND, STOPBAND);

    _dec = new FastDecimator(taps, decimatedStep);
    _am = new AmDiscriminator();

    b1 = std::exp(-2.0f * float(M_PI) * _fc / _outSampleRate);
    a0 = 1.0f - b1;

    return true;
}