#ifndef LSBDEMOD_H
#define LSBDEMOD_H

#include "core/demodulators/demodulator.h"

class LsbDemod : public Demodulator
{
public:
    LsbDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "lsb";
    }
protected:
    bool init() override;
};

#endif // LSBDEMOD_H
