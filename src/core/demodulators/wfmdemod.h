#ifndef WFMDEMOD_H
#define WFMDEMOD_H

#include <memory>

#include "core/demodulators/demodulator.h"
#include "core/fm_discriminator.h"
#include "core/fast_decimator.h"

class WfmDemod : public Demodulator
{
public:
    // First low pass
    static constexpr uint32_t PASSBAND_DEC1 = 90e3;
    static constexpr uint32_t STOPBAND_DEC1 = 120e3;

    // Second low pass
    static constexpr uint32_t PASSBAND_DEC2 = 15e3;
    static constexpr uint32_t STOPBAND_DEC2 = 19e3;

    static constexpr uint32_t MAX_DEVIATION = 75e3;

    WfmDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "wfm";
    }

protected:
    bool init() override;

private:
    std::unique_ptr<FastDecimator> _dec1, _dec2;
    std::unique_ptr<FmDiscriminator> _fm;

    float _x = 0.0f, _a0 = 0.0f, _demmph = 0.0f;

    std::vector<std::complex<float>> _iq, _o2, _o3;
    std::vector<float> _o1;
};

#endif // WFMDEMOD_H
