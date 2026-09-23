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

    if (currentDevice != nullptr) {
        qDebug() << "Changed gain to " << gain;

        currentDevice->sdr_set_rf_gain(gain);
    }

    _rfGain = gain;

    emit rfGainChanged();
}

Q_INVOKABLE void SdrBackend::setCenterFreq(quint32 freq) {
    if (_centerFrequency == freq) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) currentDevice->sdr_set_center_freq(freq);

    _centerFrequency = freq;

    emit centerFreqChanged();
}

Q_INVOKABLE void SdrBackend::setSampleRate(quint32 sampleRate) {
    if (_sampleRate == sampleRate) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) currentDevice->sdr_set_sample_rate(sampleRate);

    _sampleRate = sampleRate;
}

Q_INVOKABLE void SdrBackend::setCorrectionPPM(int correctionPPM) {
    if (_correctionPPM == correctionPPM) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) currentDevice->sdr_set_correction_ppm(correctionPPM);

    _correctionPPM = correctionPPM;

    emit correctionPPMChanged();

}

Q_INVOKABLE void SdrBackend::setShiftAmount(qint32 shiftAmount) {
    if (_shiftAmount == shiftAmount) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) currentDevice->sdr_set_shift_amount(shiftAmount);

    _shiftAmount = shiftAmount;

    emit shiftAmountChanged();
}

Q_INVOKABLE void SdrBackend::setSamplingMode(int samplingMode) {
    if (_samplingMode == samplingMode) return;

    if (samplingMode < 0 || samplingMode > 2) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_sampling_mode(static_cast<sampling_mode_t>(samplingMode));
    }

    _samplingMode = samplingMode;
}

Q_INVOKABLE void SdrBackend::setRtlAGC(bool rtlagc) {
    if (_rtlagc == rtlagc) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_rtl_agc(rtlagc);
    }

    _rtlagc = rtlagc;

    emit rtlagcChanged();
}

Q_INVOKABLE void SdrBackend::setOffsetTuning(bool offsetTuning) {
    if (_offsetTuning == offsetTuning) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_offset_tuning(offsetTuning);
    }

    _offsetTuning = offsetTuning;

    emit offsetTuningChanged();
}

Q_INVOKABLE void SdrBackend::setTunerAGC(bool tunerAGC) {
    if (_tunerAGC == tunerAGC) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_tuner_agc(tunerAGC);
    }

    _tunerAGC = tunerAGC;

    emit tunerAGCChanged();
}

Q_INVOKABLE void SdrBackend::setBiasTee(bool biasTee) {
    if (_biasTee == biasTee) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_bias_tee(biasTee);
    }

    _biasTee = biasTee;

    emit biasTeeChanged();
}

Q_INVOKABLE void SdrBackend::setShift(bool shift) {
    if (_shift == shift) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_shift(shift);
    }

    _shift = shift;

    emit shiftChanged();
}

Q_INVOKABLE void SdrBackend::setCorrectIQ(bool correctIQ) {
    if (_correctIQ == correctIQ) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_correct_IQ(correctIQ);
    }

    _correctIQ = correctIQ;

    emit correctIQChanged();
}

Q_INVOKABLE void SdrBackend::setInvertSpectrum(bool invertSpectrum) {
    if (_invertSpectrum == invertSpectrum) return;

    SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

    if (currentDevice != nullptr) {
        currentDevice->sdr_set_invert_spectrum(invertSpectrum);
    }

    _invertSpectrum = invertSpectrum;

    emit invertSpectrumChanged();
}

Q_INVOKABLE void SdrBackend::start() {
    if (_rtl) return;

    _rtl = new RtlThread(this);

    connect(_rtl, &RtlThread::spectrumReady, this, &SdrBackend::spectrum);
    connect(_rtl, &RtlThread::error, this, &SdrBackend::errorOccurred);
    connect(_rtl, &QThread::finished, this, [this] {
        _rtl->deleteLater();
        _rtl = nullptr;
        emit runningChanged();
    });

    _rtl->start();

    emit runningChanged();
}

Q_INVOKABLE void SdrBackend::stop() {
    if (_rtl) _rtl->requestInterruption();
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