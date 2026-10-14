#include "demodulator.h"

void Demodulator::process_narrow_fft(const std::complex<float>* data, size_t n, uint32_t defaultBandwidth) {
    if (!_cb) return;

    int d = _bandwidth > 0 ? static_cast<int>(std::floor(_outSampleRate / _bandwidth)) : static_cast<int>(std::floor(_outSampleRate / defaultBandwidth));

    d = std::max(1, d);

    narrowFFTOut.resize(n / d);

    for (size_t i = 0; i < narrowFFTOut.size(); ++i) narrowFFTOut[i] = data[i * d];

    _cb(narrowFFTOut.data(), narrowFFTOut.size());
}