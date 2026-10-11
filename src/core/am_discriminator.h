#ifndef AM_DISCRIMINATOR_H
#define AM_DISCRIMINATOR_H

#include <complex>
#include <vector>

class AmDiscriminator
{
public:
    AmDiscriminator() {}

    void process(const std::complex<float>* data, size_t size, std::vector<float>& out);

    void reset();
private:
    bool _hasLastSample = false;
    std::complex<float> _lastSample = 0;
};

#endif // AM_DISCRIMINATOR_H
