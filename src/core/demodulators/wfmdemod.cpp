#include "wfmdemod.h"

#include <QDebug>
#include "core/telemetry.h"

void WfmDemod::process(const std::complex<float>* in, size_t n, std::vector<float>& out) {
    if (_dirty) {
        if (init()) _dirty = false;
        else {
            qDebug() << "Demod: failed to init";

            return;
        }
    }

    Telemetry::instance().start_recording_WFMdemodProcessingTime();

    // Low pass to 240 kHz - complex
    _o1.clear();

    // Low-pass + Decimate at d steps
    _dec1->process(in, n, _o1);

    process_narrow_fft(_o1.data(), _o1.size(), 100e3);

    if (_squelch && !_squelcher.shouldSquelch(_o1.data(), _o1.size())) return;

    // FM demodulation - convert complex baseband to float audio
    // 240 kHz - real numbers
    _o2.clear();

    _fm->process(_o1.data(), _o1.size(), _o2);

    _o1.clear();

    // Convert float to std::complex<float>
    _o1.assign(_o2.begin(), _o2.end());

    _o3.clear();

    // Low-pass + Decimate
    _dec2->process(_o1.data(), _o1.size(), _o3);

    out.resize(_o3.size());

    std::transform(_o3.begin(), _o3.end(), out.begin(), [](const std::complex<float>& z) { return z.real(); });

    // Deemphasis
    for (auto& sample : out) {
        _demmph = _a0 * sample + _x * _demmph;
        sample = _demmph;
    }

    Telemetry::instance().stop_recording_WFMdemodProcessingTime();
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

    uint32_t passband = 100e3;
    uint32_t stopband = passband + passband * 0.2; // 20% more

    uint32_t monoPassband = 15e3;
    uint32_t monoStopband = monoPassband + 3e3;

    std::vector<float> taps1 = Signal::Utils::make_lowpass(_inSampleRate, passband, stopband);

    std::vector<float> taps2 = Signal::Utils::make_lowpass(midrate, monoPassband, monoStopband);

    _dec1 = std::make_unique<FastDecimator>(taps1, d1);

    _dec2 = std::make_unique<FastDecimator>(taps2, d2);

    _fm = std::make_unique<FmDiscriminator>(midrate, MAX_DEVIATION);

    // Compute a low-pass single pole IIR filter for deemphasis step
    _x = std::exp(-1.0f / (75e-6f * _outSampleRate));

    _a0 = 1.0f - _x;

    return true;
}