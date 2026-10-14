#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <complex>
#include <functional>
#include "core/squelcher.h"

class Demodulator
{
public:
    using DecimatedCallback = std::function<void(const std::complex<float>*, size_t)>;

    virtual ~Demodulator() = default;

    virtual void process(const std::complex<float>* in, size_t n, std::vector<float>& out) = 0;

    virtual void reset() = 0;

    virtual const char* name() = 0;

    void set_input_sample_rate(uint32_t sampleRate) { this->_inSampleRate = sampleRate; _dirty = true; }

    void set_output_sample_rate(uint32_t sampleRate) { this->_outSampleRate = sampleRate; _dirty = true; }

    void set_cw_shift(int shift) { this->_cwShift = shift; _dirty = true; }

    void set_bandwidth(int bandwidth) { this->_bandwidth = bandwidth; _dirty = true; }

    void set_squelch(bool squech) { this->_squelch = squech; }

    void set_squelch_amount(uint32_t squelchAmount) { this->_squelcher.setSquelch(squelchAmount); }

    void connect_decimated_callback(DecimatedCallback cb) { this->_cb = std::move(cb); }
protected:
    virtual bool init() = 0;

    uint32_t _inSampleRate = 0, _outSampleRate = 0, _squelchAmount;

    bool _squelch;

    // Only use for CW demod
    int _cwShift = 0, _bandwidth = 0;

    bool _dirty = true;

    Squelcher _squelcher;

    DecimatedCallback _cb;

    std::vector<std::complex<float>> narrowFFTOut;

    void process_narrow_fft(const std::complex<float>* data, size_t n, uint32_t defaultBandwidth);
};

#endif // DEMODULATOR_H
