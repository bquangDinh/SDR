#include "fm_discriminator.h"

void FmDiscriminator::process(const std::complex<float>* data, size_t size, std::vector<float>& out) {
    if (size == 0) return;

    out.resize(size);

    if (_hasLastSample) {
        std::complex<float> d0 = data[0] * std::conj(_lastSample);
        out[0] = std::arg(d0) * _scale;
    } else {
        out[0] = 0.0f;
    }

    for (size_t n = 1; n < size; ++n) {
        std::complex<float> d = data[n] * std::conj(data[n - 1]);

        // FM demod angle depends on max deviation and sample rate following this equation:
        // delta_angle = (2 * pi * maxDeviation * audio) / sampleRate
        // translates to "angle turn PER sample"
        // 2pi is a full turn, so multiplying it with maxDeviation returns the max angle it can turn
        out[n] = std::arg(d) * _scale;
    }

    _lastSample = data[size - 1];
    _hasLastSample = true;
}

void FmDiscriminator::reset() {
    _lastSample = 0;
    _hasLastSample = false;
}