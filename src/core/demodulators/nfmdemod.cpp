#include "nfmdemod.h"

#include "core/Signal.h"
#include <QDebug>

void NfmDemod::process(const std::complex<float>* in, size_t n, std::vector<float>& out) {
    if (_dirty) {
        if (init()) _dirty = false;
        else {
            qDebug() << "Demod: failed to init";

            return;
        }
    }

    _iq.clear();

    _dec->process(in, n, _iq);
    _fm->process(_iq.data(), _iq.size(), out);

    // Callback decimated
    if (_cb) _cb(_iq.data(), _iq.size());
}

void NfmDemod::reset() {
    // Force reset init
    _dirty = true;

    if (_dec) _dec->reset();
    if (_fm) _fm->reset();
}

bool NfmDemod::init() {
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
    _fm = new FmDiscriminator(_outSampleRate, MAX_DEVIATION);

    return true;
}