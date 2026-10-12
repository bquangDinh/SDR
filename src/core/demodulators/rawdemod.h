#ifndef RAWDEMOD_H
#define RAWDEMOD_H

#include "core/demodulators/demodulator.h"

class RawDemod : public Demodulator
{
public:
    RawDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "raw";
    }
protected:
    bool init() override;
};

#endif // RAWDEMOD_H
