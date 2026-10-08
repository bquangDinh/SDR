#include "core/sdr_backend.h"

Q_INVOKABLE void SdrBackend::setSource(int source) {
    if (_source == source) return;

    _source = source;

    // Auto fill the device list when user select a source
    refresh_devices();
}

Q_INVOKABLE void SdrBackend::setDevice(int deviceIndex) {
    sdr_config_t init_config = {
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

    if (SdrDeviceManager::instance().set_current_device(deviceIndex, init_config) < 0) {
        emit errorOccurred("Unable to initialize device");
    }
}

Q_INVOKABLE void SdrBackend::refreshDevices() {
    return refresh_devices();
}

Q_INVOKABLE void SdrBackend::setRfGain(int gain) {
    if (_rfGain == gain) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        qDebug() << "Changed gain to " << gain;

        currentDevice->sdr_set_rf_gain(gain);
    }

    _rfGain = gain;

    emit rfGainChanged();
}

Q_INVOKABLE void SdrBackend::setCenterFreq(quint32 freq) {
    if (_centerFrequency == freq) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_center_freq(freq);

    _centerFrequency = freq;

    qDebug() << "center freq is " << freq;

    emit centerFreqChanged();

    // Do the arithmetic in signed 64-bit to avoid uint32_t underflow
    qint64 half = static_cast<qint64>(_sampleRate) / 2;
    qint64 minFreqSigned = static_cast<qint64>(_centerFrequency) - half;
    qint64 maxFreqSigned = static_cast<qint64>(_centerFrequency) + half;

    // Clamp to valid range (can't have a negative frequency, and shouldn't exceed uint32_t max)
    uint32_t minFreq = static_cast<uint32_t>(std::max<qint64>(0, minFreqSigned));
    uint32_t maxFreq = static_cast<uint32_t>(std::min<qint64>(UINT32_MAX, maxFreqSigned));

    update_frequency_range(minFreq, maxFreq);
}

Q_INVOKABLE void SdrBackend::setSampleRate(quint32 sampleRate) {
    if (_sampleRate == sampleRate) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_sample_rate(sampleRate);

    if (_rtlAudioThread) _rtlAudioThread->setSampleRate(sampleRate);

    _sampleRate = sampleRate;

    // Do the arithmetic in signed 64-bit to avoid uint32_t underflow
    qint64 half = static_cast<qint64>(_sampleRate) / 2;
    qint64 minFreqSigned = static_cast<qint64>(_centerFrequency) - half;
    qint64 maxFreqSigned = static_cast<qint64>(_centerFrequency) + half;

    // Clamp to valid range (can't have a negative frequency, and shouldn't exceed uint32_t max)
    uint32_t minFreq = static_cast<uint32_t>(std::max<qint64>(0, minFreqSigned));
    uint32_t maxFreq = static_cast<uint32_t>(std::min<qint64>(UINT32_MAX, maxFreqSigned));

    update_frequency_range(minFreq, maxFreq);

    emit sampleRateChanged();
}

Q_INVOKABLE void SdrBackend::setCorrectionPPM(int correctionPPM) {
    if (_correctionPPM == correctionPPM) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_correction_ppm(correctionPPM);

    _correctionPPM = correctionPPM;

    emit correctionPPMChanged();

}

Q_INVOKABLE void SdrBackend::setShiftAmount(qint32 shiftAmount) {
    if (_shiftAmount == shiftAmount) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) currentDevice->sdr_set_shift_amount(shiftAmount);

    _shiftAmount = shiftAmount;

    emit shiftAmountChanged();
}

Q_INVOKABLE void SdrBackend::setSamplingMode(int samplingMode) {
    if (_samplingMode == samplingMode) return;

    if (samplingMode < 0 || samplingMode > 2) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_sampling_mode(static_cast<sampling_mode_t>(samplingMode));
    }

    _samplingMode = samplingMode;
}

Q_INVOKABLE void SdrBackend::setRtlAGC(bool rtlagc) {
    if (_rtlagc == rtlagc) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_rtl_agc(rtlagc);
    }

    _rtlagc = rtlagc;

    emit rtlagcChanged();
}

Q_INVOKABLE void SdrBackend::setOffsetTuning(bool offsetTuning) {
    if (_offsetTuning == offsetTuning) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_offset_tuning(offsetTuning);
    }

    _offsetTuning = offsetTuning;

    emit offsetTuningChanged();
}

Q_INVOKABLE void SdrBackend::setTunerAGC(bool tunerAGC) {
    if (_tunerAGC == tunerAGC) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_tuner_agc(tunerAGC);
    }

    _tunerAGC = tunerAGC;

    emit tunerAGCChanged();
}

Q_INVOKABLE void SdrBackend::setBiasTee(bool biasTee) {
    if (_biasTee == biasTee) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_bias_tee(biasTee);
    }

    _biasTee = biasTee;

    emit biasTeeChanged();
}

Q_INVOKABLE void SdrBackend::setShift(bool shift) {
    if (_shift == shift) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_shift(shift);
    }

    _shift = shift;

    emit shiftChanged();
}

Q_INVOKABLE void SdrBackend::setCorrectIQ(bool correctIQ) {
    if (_correctIQ == correctIQ) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_correct_IQ(correctIQ);
    }

    _correctIQ = correctIQ;

    emit correctIQChanged();
}

Q_INVOKABLE void SdrBackend::setInvertSpectrum(bool invertSpectrum) {
    if (_invertSpectrum == invertSpectrum) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr && currentDevice->initialized()) {
        currentDevice->sdr_set_invert_spectrum(invertSpectrum);
    }

    _invertSpectrum = invertSpectrum;

    emit invertSpectrumChanged();
}

Q_INVOKABLE void SdrBackend::setDemodMode(int mode) {
    if (_rtlAudioThread == nullptr) {
        qDebug() << "Audio Thread is null!";

        return;
    }

    _rtlAudioThread->setDemodMode(mode);

    _demodMode = mode;

    emit demodModeChanged();
}

Q_INVOKABLE void SdrBackend::setBandwidth(quint32 bandwidth) {
    if (_rtlAudioThread == nullptr) {
        qDebug() << "Audio Thread is null!";

        return;
    }

    _rtlAudioThread->setBandwidth(bandwidth);

    _bandwidth = bandwidth;

    emit bandwidthChanged();
}

Q_INVOKABLE void SdrBackend::setOffsetFreq(double offset) {
    qDebug() << "Offset by " << offset;

    if (_rtlAudioThread == nullptr) {
        qDebug() << "Audio Thread is null!";

        return;
    }

    _rtlAudioThread->setOffsetFreq(offset);
}

Q_INVOKABLE void SdrBackend::start() {
    if (!_rtlFFTThread) {
        qDebug() << "No FFT Thread!";

        return;
    }

    if (!_rtlAudioThread) {
        qDebug() << "No Audio Thread!";

        return;
    }

    if (!_rtlReaderThread) {
        qDebug() << "No Reader Thread!";

        return;
    }

    if (SdrDeviceManager::instance().init_current_device() < 0) {
        qDebug() << "Failed to init device";

        return;
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
    if (!_rtlFFTThread) {
        qDebug() << "No FFT Thread!";

        return;
    }

    if (!_rtlAudioThread) {
        qDebug() << "No Audio Thread!";

        return;
    }

    if (!_rtlReaderThread) {
        qDebug() << "No Reader Thread!";

        return;
    }

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

    _deviceList = list;

    emit deviceListChanged();
}

void SdrBackend::update_frequency_range(uint32_t minFreq, uint32_t maxFreq) {
    int scaleIndex = pick_scale_index(std::max(minFreq, maxFreq));

    _minFreqScaled = apply_scale(minFreq, scaleIndex);

    _maxFreqScaled = apply_scale(maxFreq, scaleIndex);

    _freqScaleLabel = QString::fromUtf8(frequency_scale_units[scaleIndex].suffix.c_str());

    emit frequencyRangeChanged();
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
    return static_cast<double>(freq) / frequency_scale_units[scaleIndex].scale;
}