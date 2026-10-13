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

    _o1.clear();

    _dec1->process(in, n, _o1);

    _o2.clear();

    _dec2->process(_o1.data(), _o1.size(), _o2);

    _fm->process(_o2.data(), _o2.size(), out);

    // Callback decimated
    if (_cb) _cb(_o2.data(), _o2.size());
}

void NfmDemod::reset() {
    // Force reset init
    _dirty = true;

    if (_dec1) _dec1->reset();
    if (_dec2) _dec2->reset();
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

    uint32_t midrate = 240e3;

    size_t d1 = _inSampleRate / midrate;
    size_t d2 = midrate / _outSampleRate;

    uint32_t passband = std::abs(_bandwidth) > 0 ? _bandwidth / 2 : PASSBAND;
    uint32_t stopband = passband + passband * 0.2; // 20% more

    std::vector<float> taps1 = Signal::Utils::make_lowpass(_inSampleRate, passband, midrate - stopband);
    std::vector<float> taps2 = Signal::Utils::make_lowpass(_outSampleRate, passband, stopband);

    _dec1 = std::make_unique<FastDecimator>(taps1, d1);
    _dec2 = std::make_unique<FastDecimator>(taps2, d2);
    _fm = std::make_unique<FmDiscriminator>(_outSampleRate, MAX_DEVIATION);

    return true;
}