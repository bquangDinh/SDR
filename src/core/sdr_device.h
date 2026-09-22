#ifndef SDR_DEVICE_H
#define SDR_DEVICE_H

#include <stddef.h>
#include <cstdint>

int sdr_init();

int sdr_set_center_freq(uint32_t centerFreq);

int sdr_set_rf_gain(int rfGain);

int sdr_get_info();

int sdr_read(void* buf, size_t n);

int sdr_destroy();

#endif // SDR_DEVICE_H
