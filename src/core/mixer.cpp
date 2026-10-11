#include "mixer.h"

#include <math.h>

Mixer::Mixer() {}

void Mixer::setOffsetFreq(double offsetFreq) {
    if (_offsetFreq == offsetFreq) return;

    _offsetFreq = offsetFreq;

    _dirty = true;
}

void Mixer::setSampleRate(uint32_t sampleRate) {
    if (_sampleRate == sampleRate) return;

    _sampleRate = sampleRate;

    _dirty = true;
}

void Mixer::compute_rotstep() {
    _rotStep = std::polar(1.0, -2.0 * M_PI * _offsetFreq / _sampleRate);

    _rot = {1.0, 0.0};
}

void Mixer::process(const std::complex<float>* data, size_t size, std::vector<std::complex<float>>& out) {
    if (_dirty) {
        compute_rotstep();

        _dirty = false;
    }

    out.resize(size);

    for (size_t i = 0; i < size; ++i) {
        out[i] = data[i] * std::complex<float>(_rot);

        _rot *= _rotStep;
    }

    _rot /= std::abs(_rot); // normalize in case the value drift due to rounding off
}

void Mixer::reset() {
    _rotStep = {1.0f, 0.0f};
    _rot = {1.0f, 0.0f};
}