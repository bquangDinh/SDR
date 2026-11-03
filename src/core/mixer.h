#ifndef MIXER_H
#define MIXER_H

#include <complex>
#include <core/Constants.h>
#include <vector>

class Mixer
{
public:
    Mixer();

    double offsetFreq() const { return this->_offsetFreq; }

    uint32_t sampleRate() const { return this->_sampleRate; }

    void set_offset_freq(double offsetFreq);

    void set_sample_rate(uint32_t sampleRate);

    void process(const std::complex<float>* data, size_t size, std::vector<std::complex<float>>& out);

    void reset();
private:
    double _offsetFreq = 0;
    uint32_t _sampleRate = Constants::SampleRate::RATE_2_4_MSPS;
    bool _dirty = true;

    std::complex<double> _rotStep = {1.0, 0.0};
    std::complex<double> _rot = {1.0, 0.0};

    void compute_rotstep();
};

#endif // MIXER_H
