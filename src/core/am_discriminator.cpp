#include "am_discriminator.h"

void AmDiscriminator::process(const std::complex<float>* data, size_t size, std::vector<float>& out) {
    if (size == 0) return;

    out.resize(size);

    if (_hasLastSample) {
        out[0] = std::abs(_lastSample);
    }

    for (size_t n = _hasLastSample ? 1 : 0; n < size; ++n) {
        out[n] = std::abs(data[n]);
    }

    _lastSample = data[size - 1];
    _hasLastSample = true;
}

void AmDiscriminator::reset() {
    _lastSample = 0;
    _hasLastSample = false;
}