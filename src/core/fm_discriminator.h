#ifndef FMDISCRIMINATOR_H
#define FMDISCRIMINATOR_H

#include <complex>
#include <vector>

class FmDiscriminator
{
public:
    FmDiscriminator(uint32_t sampleRate, uint32_t maxDeviation) :
        _sampleRate(sampleRate),
        _maxDeviation(maxDeviation)
    {
        _scale = _sampleRate / (2.0f * M_PI * _maxDeviation);
    }

    void process(const std::complex<float>* data, size_t size, std::vector<float>& out);

    void reset();
private:
    uint32_t _sampleRate, _maxDeviation;
    bool _hasLastSample = false;
    float _scale = 0.0f;
    std::complex<float> _lastSample = 0;
};

#endif // FMDISCRIMINATOR_H
