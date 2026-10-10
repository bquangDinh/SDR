#include "rtl_sdr_device.h"
#include <cassert>
#include <iostream>

int RtlSdrDevice::sdr_init(uint32_t device_index, sdr_config_t init_config) {
    if (initialized()) return 0;

    _config = init_config;

    // std::cout << "Obtaining device information..." << std::endl;

    // // Obtain device information from the driver
    // if (rtlsdr_get_device_usb_strings(
    //         device_index,
    //         _info.manufacture.data(),
    //         _info.product.data(),
    //         _info.serial.data()
    // ) < 0) {
    //     return -1;
    // }

    std::cout << "Opening the device with index " << device_index << std::endl;

    // Open the device
    if (rtlsdr_open(&handler, device_index) < 0) {
        return -1;
    }

    assert(handler != nullptr && "handler is NULL!");

    // Get the list of supported RF Gains
    // Since we don't know how many rf gain entries the device supported
    // provide nullptr to the second argument of this method will return the number of entries
    int numberRFGains = rtlsdr_get_tuner_gains(handler, nullptr);

    if (numberRFGains <= 0) {
        return -1;
    }

    std::cout << "Supported RF Gains " << numberRFGains << std::endl;

    _supported_rf_gains.resize(numberRFGains);

    if (rtlsdr_get_tuner_gains(handler, _supported_rf_gains.data()) < 0) {
        return -1;
    }

    std::cout << "Max is " << _supported_rf_gains[_supported_rf_gains.size() - 1] << std::endl;

    // Reset buffer
    if (sdr_reset() < 0) return -1;

    // Set parameters of RTL based on init config
    sdr_set_rtl_agc(init_config.rtlagc);
    sdr_set_offset_tuning(init_config.offsetTuning);
    sdr_set_tuner_agc(init_config.tunerAGC);
    sdr_set_shift(init_config.shift);
    sdr_set_correct_IQ(init_config.correctIQ);
    sdr_set_invert_spectrum(init_config.invertSpectrum);

    // Need to set shift amount first, since if shift is enabled
    // setting center frequency needs to know the shift amount
    sdr_set_shift_amount(init_config.shiftAmount);

    sdr_set_sampling_mode(init_config.samplingMode);

    sdr_set_center_freq(init_config.centerFreq);
    sdr_set_sample_rate(init_config.sampleRate);
    sdr_set_rf_gain(init_config.gain);
    sdr_set_correction_ppm(init_config.correctionPPM);

    _initialized = true;

    _destroyed = false;

    std::cout << "Initialized" << std::endl;

    return 0;
}

int RtlSdrDevice::sdr_reset() {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    return rtlsdr_reset_buffer(handler);
}

ssize_t RtlSdrDevice::sdr_read_data_sync(void* buf, size_t len) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    assert(buf != nullptr && "output buffer is not given. Make sure you allocate buffer");

    int n_read = 0;

    int rc = rtlsdr_read_sync(handler, buf, len, &n_read);

    if (rc < 0) return -1;

    return n_read;
}

ssize_t RtlSdrDevice::sdr_read_data_sync(std::vector<std::byte> &v) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    assert(!v.empty() && "the provided vector is empty");

    int n_read = 0;

    int rc = rtlsdr_read_sync(handler, v.data(), v.size(), &n_read);

    if (rc < 0) return -1;

    return n_read;
}

int RtlSdrDevice::sdr_set_center_freq(uint32_t freq) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    uint64_t actual = _config.shift ? static_cast<int64_t>(freq) + static_cast<int64_t>(_config.shiftAmount) : static_cast<int64_t>(freq);

    if (actual < static_cast<int64_t>(RtlSdrDevice::MIN_FREQ) || actual > static_cast<int64_t>(RtlSdrDevice::MAX_FREQ)) {
        return -1;
    }

    int rc = rtlsdr_set_center_freq(handler, static_cast<uint32_t>(actual));

    if (rc < 0) {
        std::cout << "Failed to set freq" << std::endl;

        return -1;
    }

    std::cout << "Set freq to " << actual << std::endl;

    _config.centerFreq = static_cast<uint32_t>(actual);

    return 0;
}

int RtlSdrDevice::sdr_set_sample_rate(uint32_t sampleRate) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (sampleRate < RtlSdrDevice::MIN_SAMPLE_RATE || sampleRate > RtlSdrDevice::MAX_SAMPLE_RATE) {
        return -1;
    }

    int rc = rtlsdr_set_sample_rate(handler, sampleRate);

    if (rc < 0) return -1;

    _config.sampleRate = sampleRate;

    return 0;
}

int RtlSdrDevice::sdr_set_rf_gain(int gain) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    assert(!_supported_rf_gains.empty() && "the list of supported rf gains is empty");

    if (_config.tunerAGC) {
        std::cout << "Tuner AGC is enabled, cannot set rf manually" << std::endl;
        return -1;
    }

    int gainTenths = gain * 10;   // convert dB -> tenths of dB, matching _supported_rf_gains' unit

    // Find the closest rf gain value from the given gain
    int best = _supported_rf_gains[0];
    int bestDiff = std::abs(gainTenths - best);

    for (int g : _supported_rf_gains) {
        int diff = std::abs(gainTenths - g);
        if (diff < bestDiff) {
            best = g;
            bestDiff = diff;
        }
    }

    int rc = rtlsdr_set_tuner_gain(handler, best);
    if (rc < 0) {
        std::cout << "Failed to set gain" << std::endl;
        return -1;
    }

    _config.gain = gain;
    _config.actualGain = best;

    std::cout << "gain is " << gain << " actual is " << best << std::endl;

    return 0;
}

int RtlSdrDevice::sdr_set_correction_ppm(int correctionPPM) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    int rc = rtlsdr_set_freq_correction(handler, correctionPPM);

    if (rc < 0) return -1;

    _config.correctionPPM = correctionPPM;

    return 0;
}

int RtlSdrDevice::sdr_set_shift_amount(int32_t shift) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    _config.shiftAmount = shift;

    return 0;
}

int RtlSdrDevice::sdr_set_sampling_mode(sampling_mode_t mode) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    int rc = rtlsdr_set_direct_sampling(handler, static_cast<int>(mode));

    if (rc < 0) return -1;

    _config.samplingMode = mode;

    return 0;
}

int RtlSdrDevice::sdr_set_rtl_agc(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.rtlagc == val) return 0;

    int rc = rtlsdr_set_agc_mode(handler, val ? 1 : 0);

    if (rc < 0) return -1;

    _config.rtlagc = val;

    return 0;
}

int RtlSdrDevice::sdr_set_offset_tuning(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.offsetTuning == val) return 0;

    int rc = rtlsdr_set_offset_tuning(handler, val ? 1 : 0);

    if (rc < 0) return -1;

    _config.offsetTuning = val;

    return 0;
}

int RtlSdrDevice::sdr_set_tuner_agc(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.tunerAGC == val) return 0;

    int rc = rtlsdr_set_tuner_gain_mode(handler, val ? 1 : 0);

    if (rc < 0) return -1;

    _config.tunerAGC = val;

    return 0;
}

int RtlSdrDevice::sdr_set_bias_tee(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.biasTee == val) return 0;

    int rc = rtlsdr_set_bias_tee(handler, val ? 1: 0);

    if (rc < 0) return -1;

    _config.biasTee = val;

    return 0;
}

int RtlSdrDevice::sdr_set_shift(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.shift == val) return 0;

    _config.shift = val;

    return 0;
}

int RtlSdrDevice::sdr_set_correct_IQ(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.correctIQ == val) return 0;

    _config.correctIQ = val;

    return 0;
}

int RtlSdrDevice::sdr_set_invert_spectrum(bool val) {
    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    if (_config.invertSpectrum == val) return 0;

    _config.invertSpectrum = val;

    return 0;
}

int RtlSdrDevice::sdr_destroy() {
    if (destroyed()) return 0;

    assert(handler != nullptr && "handler is NULL! You may forgot to call sdr_init()");

    int rc = rtlsdr_close(handler);

    handler = nullptr;

    _destroyed = true;

    _initialized = false;

    return rc;
}