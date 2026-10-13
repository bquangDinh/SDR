#include "wfmdemod.h"

#include <QDebug>

void WfmDemod::process(const std::complex<float>* in, size_t n, std::vector<float>& out) {
    if (_dirty) {
        if (init()) _dirty = false;
        else {
            qDebug() << "Demod: failed to init";

            return;
        }
    }

    _iq.clear();
    _o1.clear();
    _o2.clear();
    _o3.clear();

    // Low-pass + Decimate at d steps
    _dec1->process(in, n, _iq);

    // FM
    _fm->process(_iq.data(), _iq.size(), _o1);

    // Convert float to std::complex<float>
    _o2.assign(_o1.begin(), _o1.end());

    // Low-pass + Decimate at 5
    _dec2->process(_o2.data(), _o2.size(), _o3);

    out.resize(_o3.size());

    std::transform(_o3.begin(), _o3.end(), out.begin(), [](const std::complex<float>& z) { return z.real(); });

    // Deemphasis
    for (auto& sample : out) {
        _demmph = _a0 * sample + _x * _demmph;
        sample = _demmph;
    }

    // Callback decimated
    if (_cb) _cb(_o3.data(), _o3.size());
}

void WfmDemod::reset() {
    // Force reset init
    _dirty = true;

    _x = 0.0f;
    _a0 = 0.0f;
    _demmph = 0.0f;

    if (_dec1) _dec1->reset();
    if (_dec2) _dec2->reset();
    if (_fm) _fm->reset();
}

bool WfmDemod::init() {
    if (_inSampleRate == 0 || _outSampleRate == 0) {
        qDebug() << "Demod: You forgot to set input sample rate or output sample rate";

        return false;
    }

    if (_inSampleRate < _outSampleRate) {
        qDebug() << "Demod: Input sample rate cannot be lower than output sample rate";

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

    _fm = std::make_unique<FmDiscriminator>(midrate, MAX_DEVIATION);

    // Compute a low-pass single pole IIR filter for deemphasis step
    _x = std::exp(-1.0f / (75e-6f * _outSampleRate));

    _a0 = 1.0f - _x;

    return true;
}