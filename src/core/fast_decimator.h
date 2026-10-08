#ifndef FAST_DECIMATOR_H
#define FAST_DECIMATOR_H

#include <vector>
#include <complex>

class FastDecimator
{
public:
    FastDecimator(const std::vector<float>& taps, int d)
        : r_taps(taps.rbegin(), taps.rend()), _d(d), _bufI(taps.size() - 1, 0.0f), _bufQ(taps.size() - 1, 0.0f)
    {}

    void process(const std::complex<float>* in, size_t n, std::vector<std::complex<float>>& out);
private:
    std::vector<float> r_taps; // reverse taps
    size_t _d; // decimation step
    size_t _phase = 0;
    std::vector<float> _bufI, _bufQ;
};

#endif // FAST_DECIMATOR_H
