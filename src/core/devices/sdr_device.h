#ifndef SDR_DEVICE_H
#define SDR_DEVICE_H

#include <stddef.h>
#include <cstdint>
#include <string>
#include <vector>

struct sdr_config {
    uint32_t centerFreq;
    uint32_t sampleRate;
    int gain;
    int actualGain;
    int correctionPPM;
    int32_t shiftAmount;
    int samplingMode;
    bool rtlagc;
    bool offsetTuning;
    bool tunerAGC;
    bool biasTee;
    bool shift;
    bool correctIQ;
    bool invertSpectrum;
};

struct sdr_info {
    std::string manufacture;
    std::string product;
    std::string serial;
    std::string deviceIndex;
};

class SdrDevice {
protected:
    sdr_config _config;
    sdr_info _info;
    bool _initialized = false;
    bool _destroyed = false;
    bool running = false;

    std::vector<int> _supported_rf_gains;
public:
    virtual int sdr_init(uint32_t device_index, sdr_config init_config) = 0;

    virtual int sdr_reset() = 0;

    virtual ssize_t sdr_read_data_sync(void* buf, size_t len) = 0;

    virtual ssize_t sdr_read_data_sync(std::vector<std::byte> &v) = 0;

    const sdr_config& config() { return this->_config; }

    const sdr_info& info() { return this->_info; }

    const std::vector<int>& supported_rf_gains() { return this->_supported_rf_gains; }

    const bool initialized() { return this->_initialized; }

    const bool destroyed() { return this->_destroyed; }

    virtual int sdr_set_center_freq(uint32_t freq) = 0;

    virtual int sdr_set_sample_rate(uint32_t sampleRate) = 0;

    virtual int sdr_set_rf_gain(int gain) = 0;

    virtual int sdr_set_correction_ppm(int correctionPPM) = 0;

    virtual int sdr_set_shift_amount(int32_t shift) = 0;

    virtual int sdr_set_sampling_mode(int mode) = 0;

    virtual int sdr_set_rtl_agc(bool val) = 0;

    virtual int sdr_set_offset_tuning(bool val) = 0;

    virtual int sdr_set_tuner_agc(bool val) = 0;

    virtual int sdr_set_bias_tee(bool val) = 0;

    virtual int sdr_set_shift(bool val) = 0;

    virtual int sdr_set_correct_IQ(bool val) = 0;

    virtual int sdr_set_invert_spectrum(bool val) = 0;

    virtual int sdr_destroy() = 0;
};

#endif // SDR_DEVICE_H
