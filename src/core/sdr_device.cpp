#include "core/sdr_device.h"
#include "include/rtl-sdr.h"

static rtlsdr_dev_t* dev = NULL;

int sdr_init() {
    if (rtlsdr_open(&dev, 0) < 0) {
        return -1;
    }

    rtlsdr_set_sample_rate(dev, 2048000);
    rtlsdr_set_center_freq(dev, 100000000);   // pick any real frequency, e.g. 100 MHz
    rtlsdr_set_tuner_gain_mode(dev, 0);       // 0 = auto gain

    if (rtlsdr_reset_buffer(dev) < 0) {

        rtlsdr_close(dev);

        dev = NULL;

        return -1;
    }

    return 0;
}

int sdr_get_info() {
    return 0;
}

int sdr_read(void* buf, size_t n) {
    int bytes_read = 0;

    if (rtlsdr_read_sync(dev, buf, n, &bytes_read) < 0) {
        return -1;
    }

    return bytes_read;
}

int sdr_destroy() {
    rtlsdr_close(dev);

    return 0;
}
