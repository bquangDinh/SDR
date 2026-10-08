#include "fast_decimator.h"

void FastDecimator::process(const std::complex<float>* in, size_t n, std::vector<std::complex<float>>& out) {
    const size_t L = r_taps.size();
    const size_t H = L - 1; // size of history

    _bufI.resize(H + n); // [history | new chunk]
    _bufQ.resize(H + n); // [history | new chunk]

    // Fill new data into new chunk's section
    for (size_t i = 0; i < n; ++i) {
        _bufI[H + i] = in[i].real();
        _bufQ[H + i] = in[i].imag();
    }

    size_t j = _phase;

    for (;j < n; j += _d) {
        const float* xi = &_bufI[j];
        const float* xq = &_bufQ[j];
        const float* taps = r_taps.data();

        float ai = 0.0f, aq = 0.0f;

        for (size_t k = 0; k < L; ++k) {
            ai += taps[k] * xi[k];
            aq += taps[k] * xq[k];
        }

        out.emplace_back(ai, aq);
    }

    _phase = j - n; // carry the last phase to next chunk

    std::copy(_bufI.end() - H, _bufI.end(), _bufI.begin()); // only keep history a.k.a last H samples
    std::copy(_bufQ.end() - H, _bufQ.end(), _bufQ.begin());

    _bufI.resize(H);
    _bufQ.resize(H);
}
