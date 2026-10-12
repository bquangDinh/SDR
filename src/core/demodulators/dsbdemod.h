#ifndef DSBDEMOD_H
#define DSBDEMOD_H

#include "core/demodulators/demodulator.h"

class DsbDemod : public Demodulator
{
public:
    DsbDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "dsb";
    }
protected:
    bool init() override;
};

#endif // DSBDEMOD_H
