#ifndef RTL_SDR_DEVICE_H
#define RTL_SDR_DEVICE_H

#include "core/devices/sdr_device.h"
#include "include/rtl-sdr.h"

class RtlSdrDevice : public SdrDevice
{
private:
    rtlsdr_dev_t *handler = nullptr;
public:
    static const uint32_t MIN_FREQ = 24e6;
    static const uint32_t MAX_FREQ = 1766e6;

    static const uint32_t MIN_SAMPLE_RATE = 225001;
    static const uint32_t MAX_SAMPLE_RATE = 32e5;

    static uint32_t get_devices_count() {
        return rtlsdr_get_device_count();
    }

    static int get_device_info_from_index(uint32_t index, char* manufact, char* product, char* serial) {
        return rtlsdr_get_device_usb_strings(index, manufact, product, serial);
    }

    static const char* get_device_name_from_index(uint32_t index) {
        return rtlsdr_get_device_name(index);
    }

    int sdr_init(uint32_t device_index, sdr_config_t init_config) override;

    int sdr_reset() override;

    ssize_t sdr_read_data_sync(void* buf, size_t len) override;

    ssize_t sdr_read_data_sync(std::vector<std::byte> &v) override;

    int sdr_set_center_freq(uint32_t freq) override;

    int sdr_set_sample_rate(uint32_t sampleRate) override;

    int sdr_set_rf_gain(int gain) override;

    int sdr_set_correction_ppm(int correctionPPM) override;

    int sdr_set_shift_amount(int32_t shift) override;

    int sdr_set_sampling_mode(sampling_mode_t mode) override;

    int sdr_set_rtl_agc(bool val) override;

    int sdr_set_offset_tuning(bool val) override;

    int sdr_set_tuner_agc(bool val) override;

    int sdr_set_bias_tee(bool val) override;

    int sdr_set_shift(bool val) override;

    int sdr_set_correct_IQ(bool val) override;

    int sdr_set_invert_spectrum(bool val) override;

    int sdr_destroy() override;
};

#endif // RTL_SDR_DEVICE_H
