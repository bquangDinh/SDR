#include "core/sdr_backend.h"

#include <QtAssert>
#include "core/telemetry.h"

Q_INVOKABLE void SdrBackend::set_source(int source) {
    Q_ASSERT_X(source >= 0, "SdrBackend::setSource", "Invalid source index!");

    if (_sdr_settings_manager.source() == source) return;

    _sdr_settings_manager.set_source(source);

    // Auto fill the device list when user select a source
    refresh_devices();
}

Q_INVOKABLE void SdrBackend::set_device(int deviceIndex) {
    Q_ASSERT_X(deviceIndex >= 0, "SdrBackend::setDevice", "Invalid device index!");
    Q_ASSERT_X(deviceIndex < SdrDeviceManager::instance().devices_count(), "SdrBackend::setDevice", "Device index out of range!");
    Q_ASSERT_X(SdrDeviceManager::instance().devices_count() > 0, "SdrBackend::setDevice", "No devices available!");

    if (_sdr_settings_manager.selectedDeviceIndex() == deviceIndex) return;

    SdrDeviceManager& sdrManager = SdrDeviceManager::instance();
    
    sdr_config init_config = create_sdr_config();

    // Destroy the current device if it exists before initializing a new one
    if (!sdrManager.device_empty()) {
        sdrManager.destroy_current_device();
    }

    // Initialize the new device with the provided configuration
    if (sdrManager.init_device_with_config(deviceIndex, init_config) < 0) {
        emit errorOccurred("Unable to initialize device");
    } else {
        _sdr_settings_manager.set_selected_device_index(deviceIndex);
    }
}

Q_INVOKABLE void SdrBackend::refresh_devices() {
    if (source() == -1) return;

    DEVICE_SOURCE s = static_cast<DEVICE_SOURCE>(source());

    if (SdrDeviceManager::instance().scan_devices(s) < 0) {
        return;
    }

    std::vector<device_entry> devices = SdrDeviceManager::instance().availableDevices();

    qDebug() << "Number of devices: " << devices.size();

    if (devices.empty()) return;

    QVariantList list;

    for (const device_entry &d : devices) {
        QVariantMap entry;

        entry["text"] = QString(QString::fromStdString(d.deviceName));
        entry["index"] = d.deviceIndex;

        list.append(entry);
    }

    _deviceList = list;

    // // Auto init the first one in the list
    // sdr_config_t config = create_sdr_config();

    // SdrDeviceManager::instance().init_device_with_config(devices[0].deviceIndex, config);

    emit deviceListChanged();
}

Q_INVOKABLE void SdrBackend::set_rf_gain(int gain) {
    Q_ASSERT_X(gain >= 0, "SdrBackend::setRfGain", "Invalid gain value!");

    if (_sdr_settings_manager.rfGain() == gain) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_rf_gain(gain);
    }

    _sdr_settings_manager.set_rf_gain(gain);

    emit rfGainChanged();
}

Q_INVOKABLE void SdrBackend::set_center_freq(quint32 freq) {
    if (_sdr_settings_manager.centerFreq() == freq) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_center_freq(freq);

    _sdr_settings_manager.set_center_freq(freq);

    update_frequency_range();

    emit centerFreqChanged();
}

Q_INVOKABLE void SdrBackend::set_sample_rate(quint32 sampleRate) {
    if (_sdr_settings_manager.sampleRate() == sampleRate) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_sample_rate(sampleRate);

    _sdr_settings_manager.set_sample_rate(sampleRate);

    update_frequency_range();

    emit sampleRateChanged();
}

Q_INVOKABLE void SdrBackend::set_correction_ppm(int correctionPPM) {
    Q_ASSERT_X(correctionPPM >= -100 && correctionPPM <= 100, "SdrBackend::setCorrectionPPM", "Invalid correction PPM value!");

    if (_sdr_settings_manager.correctionPPM() == correctionPPM) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_correction_ppm(correctionPPM);

    _sdr_settings_manager.set_correction_ppm(correctionPPM);

    emit correctionPPMChanged();
}

Q_INVOKABLE void SdrBackend::set_shift_amount(qint32 shiftAmount) {
    if (_sdr_settings_manager.shiftAmount() == shiftAmount) return;

    // SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    // if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_shift_amount(shiftAmount);

    _sdr_settings_manager.set_shift_amount(shiftAmount);

    emit shiftAmountChanged();
}

Q_INVOKABLE void SdrBackend::set_sampling_mode(int samplingMode) {
    Q_ASSERT_X(samplingMode >= Constants::SamplingMode::QUADRATURE_SAMPLING && samplingMode <= Constants::SamplingMode::DIRECT_SAMPLING_Q_BRANCH, "SdrBackend::setSamplingMode", "Invalid sampling mode!");
    
    if (_sdr_settings_manager.samplingMode() == samplingMode) return;

    if (samplingMode < Constants::SamplingMode::QUADRATURE_SAMPLING || samplingMode > Constants::SamplingMode::DIRECT_SAMPLING_Q_BRANCH) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_sampling_mode(samplingMode);
    }

    _sdr_settings_manager.set_sampling_mode(samplingMode);
}

Q_INVOKABLE void SdrBackend::set_rtl_agc(bool rtlagc) {
    if (_sdr_settings_manager.rtlAgc() == rtlagc) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_rtl_agc(rtlagc);
    }

    _sdr_settings_manager.set_rtl_agc(rtlagc);

    emit rtlagcChanged();
}

Q_INVOKABLE void SdrBackend::set_offset_tuning(bool offsetTuning) {
    if (_sdr_settings_manager.offsetTuning() == offsetTuning) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_offset_tuning(offsetTuning);
    }

    _sdr_settings_manager.set_offset_tuning(offsetTuning);

    emit offsetTuningChanged();
}

Q_INVOKABLE void SdrBackend::set_tuner_agc(bool tunerAGC) {
    if (_sdr_settings_manager.tunerAgc() == tunerAGC) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_tuner_agc(tunerAGC);
    }

    _sdr_settings_manager.set_tuner_agc(tunerAGC);

    emit tunerAGCChanged();
}

Q_INVOKABLE void SdrBackend::set_bias_tee(bool biasTee) {
    if (_sdr_settings_manager.biasTee() == biasTee) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_bias_tee(biasTee);
    }

    _sdr_settings_manager.set_bias_tee(biasTee);

    emit biasTeeChanged();
}

Q_INVOKABLE void SdrBackend::set_shift(bool shift) {
    if (_sdr_settings_manager.shift() == shift) return;

    // SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    // if (currentDevice != nullptr && currentDevice->initialized()) {
    //     currentDevice->sdr_set_shift(shift);
    // }

    _sdr_settings_manager.set_shift(shift);

    emit shiftChanged();
}

Q_INVOKABLE void SdrBackend::set_correct_iq(bool correctIQ) {
    if (_sdr_settings_manager.correctIQ() == correctIQ) return;

    _sdr_settings_manager.set_correct_iq(correctIQ);

    emit correctIQChanged();
}

Q_INVOKABLE void SdrBackend::set_invert_spectrum(bool invertSpectrum) {
    if (_sdr_settings_manager.invertSpectrum() == invertSpectrum) return;

    _sdr_settings_manager.set_invert_spectrum(invertSpectrum);

    emit invertSpectrumChanged();
}

Q_INVOKABLE void SdrBackend::set_demod_mode(int mode) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBandwidth", "RTL Audio Thread is not defined!");
    Q_ASSERT_X(mode >= 0 && mode <= 7, "sdr_backend::setDemodMode", "demodulation mode value is not valid");

    if (_sdr_settings_manager.demodMode() == mode) return;

    _sdr_settings_manager.set_demod_mode(mode);

    emit demodModeChanged();
}

Q_INVOKABLE void SdrBackend::set_bandwidth(quint32 bandwidth) {    
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBandwidth", "RTL Audio Thread is not defined!");

    Q_ASSERT_X(bandwidth > 0, "sdr_backend::setBandwidth", "bandwidth must be greater than 0");

    if (_sdr_settings_manager.bandwidth() == bandwidth) return;

    _sdr_settings_manager.set_bandwidth(bandwidth);

    emit bandwidthChanged();
}

Q_INVOKABLE void SdrBackend::set_squelch(bool squelch) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBandwidth", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.squelch() == squelch) return;

    _sdr_settings_manager.set_squelch(squelch);

    emit squelchChanged();
}

Q_INVOKABLE void SdrBackend::set_squelch_amount(quint32 squelchAmount) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setSquelchAmount", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.squelchAmount() == squelchAmount) return;

    _sdr_settings_manager.set_squelch_amount(squelchAmount);

    emit squelchAmountChanged();
}

Q_INVOKABLE void SdrBackend::set_cw_shift_amount(quint32 cwShiftAmount) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setCWShiftAmount", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.cwShiftAmount() == cwShiftAmount) return;

    _sdr_settings_manager.set_cw_shift_amount(cwShiftAmount);

    emit cwShiftAmountChanged();
}

Q_INVOKABLE void SdrBackend::set_binaural(bool binaural) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBinaural", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.binaural() == binaural) return;

    _sdr_settings_manager.set_binaural(binaural);

    emit binauralChanged();
}

Q_INVOKABLE void SdrBackend::set_anti_fading(bool antiFading) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setAntiFading", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.antiFading() == antiFading) return;

    _sdr_settings_manager.set_anti_fading(antiFading);

    emit antiFadingChanged();
}

Q_INVOKABLE void SdrBackend::set_lock_carrier(bool lockCarrier) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setLockCarrier", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.lockCarrier() == lockCarrier) return;

    _sdr_settings_manager.set_lock_carrier(lockCarrier);

    emit lockCarrierChanged();
}

Q_INVOKABLE void SdrBackend::set_sticky_lock(bool stickyLock) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setStickyLock", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.stickyLock() == stickyLock) return;

    _sdr_settings_manager.set_sticky_lock(stickyLock);

    emit stickyLockChanged();
}

Q_INVOKABLE void SdrBackend::set_offset_freq(double offset) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setOffsetFreq", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.offsetFreq() == offset) return;

    _sdr_settings_manager.set_offset_freq(offset);

    emit offsetFreqChanged();
}

Q_INVOKABLE void SdrBackend::set_volume(int volume) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::set_volume", "RTL Audio Thread is not defined!");

    if (_sdr_settings_manager.volume() == volume) return;

    _sdr_settings_manager.set_volume(volume);

    emit volumeChanged();
}

Q_INVOKABLE void SdrBackend::start() {
    Q_ASSERT_X(_rtlFFTThread != nullptr, "sdr_backend::start", "FFT Thread is not defined!");
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::start", "Audio Thread is not defined!");
    Q_ASSERT_X(_rtlReaderThread != nullptr, "sdr_backend::start", "Reader Thread is not defined!");

    // if (SdrDeviceManager::instance().device_empty()) {
    //     sdr_config_t config = create_sdr_config();

    //     if (SdrDeviceManager::instance().init_device_with_config(_sdr_settings_manager.selectedDeviceIndex(), config) < 0) {
    //         return;
    //     }
    // }

    if (!_threadsStart) {
        _rtlFFTThread->start();

        _rtlAudioThread->start();

        _rtlReaderThread->start();

        _threadsStart = true;
    } else {
        _rtlFFTThread->resume();

        _rtlAudioThread->resume();

        _rtlReaderThread->resume();
    }

    _running = true;

    // UI Change
    emit runningChanged();
}

Q_INVOKABLE void SdrBackend::stop() {
    Q_ASSERT_X(_rtlFFTThread != nullptr, "sdr_backend::stop", "FFT Thread is not defined!");
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::stop", "Audio Thread is not defined!");
    Q_ASSERT_X(_rtlReaderThread != nullptr, "sdr_backend::stop", "Reader Thread is not defined!");

    _rtlFFTThread->pause();

    _rtlAudioThread->pause();

    _rtlReaderThread->pause();

    // SdrDeviceManager::instance().destroy_current_device();

    _running = false;

    // Print telemetry report
    Telemetry::instance().print_report();

    // UI Change
    emit runningChanged();
}

void SdrBackend::update_frequency_range() {


    // Do the arithmetic in signed 64-bit to avoid uint32_t underflow
    qint64 half = static_cast<qint64>(_sdr_settings_manager.sampleRate()) / 2;
    qint64 minFreqSigned = static_cast<qint64>(_sdr_settings_manager.centerFreq()) - half;
    qint64 maxFreqSigned = static_cast<qint64>(_sdr_settings_manager.centerFreq()) + half;

    // Clamp to valid range (can't have a negative frequency, and shouldn't exceed uint32_t max)
    uint32_t minFreq = static_cast<uint32_t>(std::max<qint64>(0, minFreqSigned));
    uint32_t maxFreq = static_cast<uint32_t>(std::min<qint64>(UINT32_MAX, maxFreqSigned));

    int scaleIndex = pick_scale_index(std::max(minFreq, maxFreq));

    _minFreqScaled = apply_scale(minFreq, scaleIndex);

    _maxFreqScaled = apply_scale(maxFreq, scaleIndex);

    _freqScaleLabel = QString::fromUtf8(frequency_scale_units[scaleIndex].suffix.c_str());

    _freqScale = frequency_scale_units[scaleIndex].scale;

    emit frequencyRangeChanged();

    emit freqScaleChanged();
}

int SdrBackend::pick_scale_index(uint32_t freq) {
    int i = 0;

    while (
        i < static_cast<int>(frequency_scale_units.size()) - 1 &&
        freq >= frequency_scale_units[i + 1].scale
    ) {
        i++;
    }

    double scaled = static_cast<double>(freq) / frequency_scale_units[i].scale;

    if (std::ceil(scaled) >= 1000 && i <= static_cast<int>(frequency_scale_units.size()) - 1) {
        i++;
    }

    return i;
}

double SdrBackend::apply_scale(uint32_t freq, int scaleIndex) {
    Q_ASSERT_X(scaleIndex >= 0 && scaleIndex < static_cast<int>(frequency_scale_units.size()), "sdr_backend::apply_scale", "Invalid scale index!");

    return static_cast<double>(freq) / frequency_scale_units[scaleIndex].scale;
}

void SdrBackend::init() {   
    update_frequency_range();
}

sdr_config SdrBackend::create_sdr_config() {
    sdr_config config = {
        .centerFreq = centerFreq(),
        .sampleRate = sampleRate(),
        .gain = rfGain(),
        .correctionPPM = correctionPPM(),
        .shiftAmount = shiftAmount(),
        .samplingMode = samplingMode(),
        .rtlagc = rtlagc(),
        .offsetTuning = offsetTuning(),
        .tunerAGC = tunerAGC(),
        .biasTee = biasTee(),
        .shift = shift(),
        .correctIQ = correctIQ(),
        .invertSpectrum = invertSpectrum()
    };

    return config;
}