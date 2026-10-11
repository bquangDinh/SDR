#ifndef NFMDEMOD_H
#define NFMDEMOD_H

#include "core/demodulators/demodulator.h"
#include "core/fast_decimator.h"
#include "core/fm_discriminator.h"

class NfmDemod : public Demodulator
{
public:
    static constexpr uint32_t PASSBAND = 8e3;
    static constexpr uint32_t STOPBAND = 24e3;
    static constexpr uint32_t MAX_DEVIATION = 5e3;

    NfmDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "nfm";
    }
protected:
    bool init() override;
private:
    FastDecimator* _dec = nullptr;
    FmDiscriminator* _fm = nullptr;

    std::vector<std::complex<float>> _iq;

};

#endif // NFMDEMOD_H
