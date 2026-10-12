#ifndef USBDEMOD_H
#define USBDEMOD_H

#include "core/demodulators/demodulator.h"

class UsbDemod : public Demodulator
{
public:
    UsbDemod() {}

    void process(const std::complex<float>* in, size_t n, std::vector<float>& out) override;

    void reset() override;

    const char* name() override {
        return "usb";
    }
protected:
    bool init() override;
};

#endif // USBDEMOD_H
