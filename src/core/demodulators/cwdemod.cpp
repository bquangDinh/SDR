#include "cwdemod.h"

#include <QDebug>
#include "core/Signal.h"

void CwDemod::process(const std::complex<float>* in, size_t n, std::vector<float>& out) {
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

    // Callback decimated
    if (_cb) _cb(_o1.data(), _o1.size());

    _o1.clear();

    _mixer->process(_o2.data(), _o2.size(), _o1);

    _o2.assign(_o1.size() * upsampleSteps, 0);

    for (size_t i = 0; i < _o1.size(); ++i) _o2[i * upsampleSteps] = _o1[i];

    _o1.clear();

    _dec3->process(_o2.data(), _o2.size(), _o1);

    out.resize(_o1.size());

    for (size_t i = 0; i < _o1.size(); ++i) out[i] = std::real(_o1[i]);
}

void CwDemod::reset() {
    // Force reset init
    _dirty = true;

    if (_dec1) _dec1->reset();
    if (_dec2) _dec2->reset();
    if (_dec3) _dec3->reset();
    if (_mixer) _mixer->reset();
}

bool CwDemod::init() {
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

    const uint32_t midrate = 12e3;
    const int d = _inSampleRate / midrate;
    upsampleSteps = _outSampleRate / midrate;

    const uint32_t fMax = std::abs(this->_cwShift) + 250; // assume 250 is the bandwidth

    std::vector<float> taps1 = Signal::Utils::make_lowpass(_inSampleRate, PASSBAND_DEC1, STOPBAND_DEC1);

    std::vector<float> taps2 = Signal::Utils::make_lowpass(midrate, PASSBAND_DEC2, STOPBAND_DEC2);

    std::vector<float> taps3 = Signal::Utils::make_lowpass(_outSampleRate, fMax, midrate - fMax);

    for (auto& t : taps3) t *= upsampleSteps;

    _dec1 = std::make_unique<FastDecimator>(taps1, d);

    _dec2 = std::make_unique<FastDecimator>(taps2, 1);

    _dec3 = std::make_unique<FastDecimator>(taps3, 1);

    _mixer = std::make_unique<Mixer>();

    _mixer->setOffsetFreq(this->_cwShift);

    _mixer->setSampleRate(midrate);

    return true;
}