#ifndef AMDEMOD_H
#define AMDEMOD_H

#include "core/demodulators/demodulator.h"
#include "core/fast_decimator.h"
#include "core/am_discriminator.h"

class AMDemod : public Demodulator
{
public:
    static constexpr uint32_t PASSBAND = 8e3;
    static constexpr uint32_t STOPBAND = 24e3;

    AMDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "am";
    }

protected:
    bool init() override;

private:
    FastDecimator* _dec = nullptr;
    AmDiscriminator* _am = nullptr;
    float _carrier = 0;
    const float _fc = 30.0f;
    float b1, a0;

    std::vector<std::complex<float>> _iq;
};

#endif // AMDEMOD_H
