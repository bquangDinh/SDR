#ifndef CWDEMOD_H
#define CWDEMOD_H

#include <memory>

#include "core/demodulators/demodulator.h"
#include "core/fast_decimator.h"
#include "core/mixer.h"

class CwDemod : public Demodulator
{
public:
    static constexpr uint32_t PASSBAND_DEC1 = 250;
    static constexpr uint32_t STOPBAND_DEC1 = 11750;

    static constexpr uint32_t PASSBAND_DEC2 = 250;
    static constexpr uint32_t STOPBAND_DEC2 = 350;

    CwDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "cw";
    }
protected:
    bool init() override;

private:
    std::unique_ptr<FastDecimator> _dec1, _dec2, _dec3;
    std::unique_ptr<Mixer> _mixer;

    std::vector<std::complex<float>> _o1, _o2;
    uint32_t upsampleSteps = 0;
};

#endif // CWDEMOD_H
