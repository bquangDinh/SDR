#include "demodulator.h"

void Demodulator::demodulate_nfm(const std::complex<float>* data, size_t size, std::vector<uint16_t>& out) {
    assert(_sampleRate > 0 && "Sample rate must larger than zero");
    assert(_audioSampleRate > 0 && "Audio sample rate must larger than zero");
    assert(_decimatedStep > 0 && "Decimated step must larger than zero");

    if (size == 0) return;
}