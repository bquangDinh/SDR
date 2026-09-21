#ifndef SDR_DEVICE_H
#define SDR_DEVICE_H

#include <stddef.h>

int sdr_init();

int sdr_get_info();

int sdr_read(void* buf, size_t n);

int sdr_destroy();

#endif // SDR_DEVICE_H
