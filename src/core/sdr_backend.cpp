#include "core/sdr_backend.h"

#include <QtAssert>

Q_INVOKABLE void SdrBackend::setSource(int source) {
    Q_ASSERT_X(source >= 0, "SdrBackend::setSource", "Invalid source index!");

    if (_device_settings.source == source) return;

    _device_settings.source = source;

    // Auto fill the device list when user select a source
    refresh_devices();
}

Q_INVOKABLE void SdrBackend::setDevice(int deviceIndex) {
    Q_ASSERT_X(deviceIndex >= 0, "SdrBackend::setDevice", "Invalid device index!");
    Q_ASSERT_X(deviceIndex < SdrDeviceManager::instance().devices_count(), "SdrBackend::setDevice", "Device index out of range!");
    Q_ASSERT_X(SdrDeviceManager::instance().devices_count() > 0, "SdrBackend::setDevice", "No devices available!");

    if (_device_settings.selectedDeviceIndex == deviceIndex) return;

    sdr_config_t init_config = create_sdr_config();

    if (SdrDeviceManager::instance().set_current_device(deviceIndex, init_config) < 0) {
        emit errorOccurred("Unable to initialize device");
    } else {
         _device_settings.selectedDeviceIndex = deviceIndex;
    }
}

Q_INVOKABLE void SdrBackend::refreshDevices() {
    return refresh_devices();
}

Q_INVOKABLE void SdrBackend::setRfGain(int gain) {
    Q_ASSERT_X(gain >= 0, "SdrBackend::setRfGain", "Invalid gain value!");

    if (_device_settings.rfGain == gain) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        qDebug() << "Changed gain to " << gain;

        currentDevice->sdr_set_rf_gain(gain);
    }

    _device_settings.rfGain = gain;

    emit rfGainChanged();
}

Q_INVOKABLE void SdrBackend::setCenterFreq(quint32 freq) {
    if (_device_settings.centerFreq == freq) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_center_freq(freq);

    _device_settings.centerFreq = freq;

    update_frequency_range();

    emit centerFreqChanged();
}

Q_INVOKABLE void SdrBackend::setSampleRate(quint32 sampleRate) {
    if (_device_settings.sampleRate == sampleRate) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_sample_rate(sampleRate);

    if (_rtlAudioThread) _rtlAudioThread->setSampleRate(sampleRate);

    if (_rtlReaderThread) _rtlReaderThread->setSampleRate(sampleRate);

    _device_settings.sampleRate = sampleRate;

    update_frequency_range();

    emit sampleRateChanged();
}

Q_INVOKABLE void SdrBackend::setCorrectionPPM(int correctionPPM) {
    Q_ASSERT_X(correctionPPM >= -100 && correctionPPM <= 100, "SdrBackend::setCorrectionPPM", "Invalid correction PPM value!");

    if (_device_settings.correctionPPM == correctionPPM) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_correction_ppm(correctionPPM);

    _device_settings.correctionPPM = correctionPPM;

    emit correctionPPMChanged();
}

Q_INVOKABLE void SdrBackend::setShiftAmount(qint32 shiftAmount) {
    if (_device_settings.shiftAmount == shiftAmount) return;

    // SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    // if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_shift_amount(shiftAmount);

    _device_settings.shiftAmount = shiftAmount;

    emit shiftAmountChanged();
}

Q_INVOKABLE void SdrBackend::setSamplingMode(int samplingMode) {
    Q_ASSERT_X(samplingMode >= 0 && samplingMode <= 2, "SdrBackend::setSamplingMode", "Invalid sampling mode!");
    
    if (_device_settings.samplingMode == samplingMode) return;

    if (samplingMode < 0 || samplingMode > 2) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_sampling_mode(static_cast<sampling_mode_t>(samplingMode));
    }

    _device_settings.samplingMode = samplingMode;
}

Q_INVOKABLE void SdrBackend::setRtlAGC(bool rtlagc) {
    if (_device_settings.rtlAgc == rtlagc) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_rtl_agc(rtlagc);
    }

    _device_settings.rtlAgc = rtlagc;

    emit rtlagcChanged();
}

Q_INVOKABLE void SdrBackend::setOffsetTuning(bool offsetTuning) {
    if (_device_settings.offsetTuning == offsetTuning) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_offset_tuning(offsetTuning);
    }

    _device_settings.offsetTuning = offsetTuning;

    emit offsetTuningChanged();
}

Q_INVOKABLE void SdrBackend::setTunerAGC(bool tunerAGC) {
    if (_device_settings.tunerAgc == tunerAGC) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_tuner_agc(tunerAGC);
    }

    _device_settings.tunerAgc = tunerAGC;

    emit tunerAGCChanged();
}

Q_INVOKABLE void SdrBackend::setBiasTee(bool biasTee) {
    if (_device_settings.biasTee == biasTee) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_bias_tee(biasTee);
    }

    _device_settings.biasTee = biasTee;

    emit biasTeeChanged();
}

Q_INVOKABLE void SdrBackend::setShift(bool shift) {
    if (_device_settings.shift == shift) return;

    // SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    // if (currentDevice != nullptr && currentDevice->initialized()) {
    //     currentDevice->sdr_set_shift(shift);
    // }

    _device_settings.shift = shift;

    emit shiftChanged();
}

Q_INVOKABLE void SdrBackend::setCorrectIQ(bool correctIQ) {
    if (_device_settings.correctIQ == correctIQ) return;

    // SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    // if (currentDevice != nullptr && currentDevice->initialized()) {
    //     currentDevice->sdr_set_correct_IQ(correctIQ);
    // }

    _device_settings.correctIQ = correctIQ;

    if (_rtlReaderThread) _rtlReaderThread->setCorrectIQ(correctIQ);

    emit correctIQChanged();
}

Q_INVOKABLE void SdrBackend::setInvertSpectrum(bool invertSpectrum) {
    if (_device_settings.invertSpectrum == invertSpectrum) return;

    // SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    // if (currentDevice != nullptr && currentDevice->initialized()) {
    //     currentDevice->sdr_set_invert_spectrum(invertSpectrum);
    // }

    if (_rtlReaderThread) _rtlReaderThread->setInvertSpectrum(invertSpectrum);

    _device_settings.invertSpectrum = invertSpectrum;

    emit invertSpectrumChanged();
}

Q_INVOKABLE void SdrBackend::setDemodMode(int mode) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBandwidth", "RTL Audio Thread is not defined!");
    Q_ASSERT_X(mode >= 0 && mode <= 7, "sdr_backend::setDemodMode", "demodulation mode value is not valid");

    if (_radio_settings.demodMode == mode) return;

    _rtlAudioThread->setDemodMode(mode);

    _radio_settings.demodMode = mode;

    emit demodModeChanged();
}

Q_INVOKABLE void SdrBackend::setBandwidth(quint32 bandwidth) {    
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBandwidth", "RTL Audio Thread is not defined!");

    Q_ASSERT_X(bandwidth > 0, "sdr_backend::setBandwidth", "bandwidth must be greater than 0");

    if (_radio_settings.bandwidth == bandwidth) return;

    if (_rtlAudioThread) _rtlAudioThread->setBandwidth(bandwidth);

    if (_rtlFFTThread) _rtlFFTThread->set_narrow_display_rate(bandwidth <= 48e3 ? bandwidth : 48e3);

    _radio_settings.bandwidth = bandwidth;

    emit bandwidthChanged();
}

Q_INVOKABLE void SdrBackend::setSquelch(bool squelch) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBandwidth", "RTL Audio Thread is not defined!");

    if (_radio_settings.squelch == squelch) return;

    _radio_settings.squelch = squelch;

    if (_rtlAudioThread) _rtlAudioThread->setSquelch(squelch);

    emit squelchChanged();
}

Q_INVOKABLE void SdrBackend::setSquelchAmount(quint32 squelchAmount) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setSquelchAmount", "RTL Audio Thread is not defined!");

    if (_radio_settings.squelchAmount == squelchAmount) return;

    _radio_settings.squelchAmount = squelchAmount;

    if (_rtlAudioThread) _rtlAudioThread->setSquelchAmount(squelchAmount);

    emit squelchAmountChanged();
}

Q_INVOKABLE void SdrBackend::setCWShiftAmount(quint32 cwShiftAmount) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setCWShiftAmount", "RTL Audio Thread is not defined!");

    if (_radio_settings.cwShiftAmount == cwShiftAmount) return;

    _rtlAudioThread->setCWShiftAmount(cwShiftAmount);

    _radio_settings.cwShiftAmount = cwShiftAmount;

    emit cwShiftAmountChanged();
}

Q_INVOKABLE void SdrBackend::setBinaural(bool binaural) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setBinaural", "RTL Audio Thread is not defined!");

    if (_radio_settings.binaural == binaural) return;

    // _rtlAudioThread->setBinaural(binaural);

    _radio_settings.binaural = binaural;

    emit binauralChanged();
}

Q_INVOKABLE void SdrBackend::setAntiFading(bool antiFading) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setAntiFading", "RTL Audio Thread is not defined!");

    if (_radio_settings.antiFading == antiFading) return;

    // _rtlAudioThread->setAntiFading(antiFading);

    _radio_settings.antiFading = antiFading;

    emit antiFadingChanged();
}

Q_INVOKABLE void SdrBackend::setLockCarrier(bool lockCarrier) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setLockCarrier", "RTL Audio Thread is not defined!");

    if (_radio_settings.lockCarrier == lockCarrier) return;

    // _rtlAudioThread->setLockCarrier(lockCarrier);

    _radio_settings.lockCarrier = lockCarrier;

    emit lockCarrierChanged();
}

Q_INVOKABLE void SdrBackend::setStickyLock(bool stickyLock) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setStickyLock", "RTL Audio Thread is not defined!");

    if (_radio_settings.stickyLock == stickyLock) return;

    // _rtlAudioThread->setStickyLock(stickyLock);

    _radio_settings.stickyLock = stickyLock;

    emit stickyLockChanged();
}

Q_INVOKABLE void SdrBackend::setOffsetFreq(double offset) {
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::setOffsetFreq", "RTL Audio Thread is not defined!");

    _rtlAudioThread->setOffsetFreq(offset);
}

Q_INVOKABLE void SdrBackend::start() {
    Q_ASSERT_X(_rtlFFTThread != nullptr, "sdr_backend::start", "FFT Thread is not defined!");
    Q_ASSERT_X(_rtlAudioThread != nullptr, "sdr_backend::start", "Audio Thread is not defined!");
    Q_ASSERT_X(_rtlReaderThread != nullptr, "sdr_backend::start", "Reader Thread is not defined!");

    if (SdrDeviceManager::instance().device_empty()) {
        sdr_config_t config = create_sdr_config();

        if (SdrDeviceManager::instance().init_device_with_config(_device_settings.selectedDeviceIndex, config) < 0) {
            return;
        }
    } else {
        if (SdrDeviceManager::instance().init_current_device() < 0) {
            return;
        }
    }

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

    SdrDeviceManager::instance().destroy_current_device();

    _running = false;

    // UI Change
    emit runningChanged();
}

void SdrBackend::refresh_devices() {
    if (source() == -1) return;

    device_source_t s = static_cast<device_source_t>(source());

    if (SdrDeviceManager::instance().scan_devices(s) < 0) {
        return;
    }

    std::vector<device_entry_t> devices = SdrDeviceManager::instance().availableDevices();

    qDebug() << "Number of devices: " << devices.size();

    if (devices.empty()) return;

    QVariantList list;

    for (const device_entry_t &d : devices) {
        QVariantMap entry;

        entry["text"] = QString(QString::fromStdString(d.deviceName));
        entry["index"] = d.deviceIndex;

        list.append(entry);
    }

    _device_settings.deviceList = list;

    // // Auto init the first one in the list
    // sdr_config_t config = create_sdr_config();

    // SdrDeviceManager::instance().init_device_with_config(devices[0].deviceIndex, config);

    emit deviceListChanged();
}

void SdrBackend::update_frequency_range() {
    // Do the arithmetic in signed 64-bit to avoid uint32_t underflow
    qint64 half = static_cast<qint64>(_device_settings.sampleRate) / 2;
    qint64 minFreqSigned = static_cast<qint64>(_device_settings.centerFreq) - half;
    qint64 maxFreqSigned = static_cast<qint64>(_device_settings.centerFreq) + half;

    // Clamp to valid range (can't have a negative frequency, and shouldn't exceed uint32_t max)
    uint32_t minFreq = static_cast<uint32_t>(std::max<qint64>(0, minFreqSigned));
    uint32_t maxFreq = static_cast<uint32_t>(std::min<qint64>(UINT32_MAX, maxFreqSigned));

    int scaleIndex = pick_scale_index(std::max(minFreq, maxFreq));

    _device_settings.minFreqScaled = apply_scale(minFreq, scaleIndex);

    _device_settings.maxFreqScaled = apply_scale(maxFreq, scaleIndex);

    _device_settings.freqScaleLabel = QString::fromUtf8(frequency_scale_units[scaleIndex].suffix.c_str());

    _device_settings.freqScale = frequency_scale_units[scaleIndex].scale;

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

void SdrBackend::set_defaults() {
    _device_settings.source = -1;
    _device_settings.selectedDeviceIndex = -1;
    _device_settings.centerFreq = 162475000;
    _device_settings.sampleRate = Constants::SampleRate::RATE_2_4_MSPS;
    _device_settings.samplingMode = QUADRATURE_SAMPLING;
    _device_settings.rfGain = 0;
    _device_settings.correctionPPM = 0;
    _device_settings.shiftAmount = 0;
    _device_settings.rtlAgc = false;
    _device_settings.offsetTuning = false;
    _device_settings.tunerAgc = false;
    _device_settings.biasTee = false;
    _device_settings.shift = false;
    _device_settings.correctIQ = false;
    _device_settings.invertSpectrum = false;

    update_frequency_range();

    _radio_settings.demodMode = Constants::DemodulateMode::NFM;
    _radio_settings.bandwidth = 8000;
    _radio_settings.squelch = false;
    _radio_settings.squelchAmount = 0;
    _radio_settings.cwShiftAmount = 0;
    _radio_settings.binaural = false;
    _radio_settings.antiFading = false;
    _radio_settings.lockCarrier = false;
    _radio_settings.stickyLock = false;

    if (_rtlFFTThread) _rtlFFTThread->set_narrow_display_rate(_radio_settings.bandwidth);
}

sdr_config_t SdrBackend::create_sdr_config() {
    sdr_config_t config = {
        .centerFreq = centerFreq(),
        .sampleRate = sampleRate(),
        .gain = rfGain(),
        .correctionPPM = correctionPPM(),
        .shiftAmount = shiftAmount(),
        .samplingMode = static_cast<sampling_mode_t>(samplingMode()),
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