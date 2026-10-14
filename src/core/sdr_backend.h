#ifndef SDR_BACKEND_H
#define SDR_BACKEND_H

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QDebug>
#include <QVariant>

#include "core/Constants.h"
#include "core/sdr_device_manager.h"
#include "core/rtl_reader_thread.h"
#include "core/rtl_audio_thread.h"
#include "core/rtl_fft_thread.h"

typedef struct frequency_scale_unit {
    double scale;
    std::string suffix;
} frequency_scale_unit_t;

typedef struct device_settings {
    int source;
    QVariantList deviceList;
    QString freqScaleLabel;
    int selectedDeviceIndex;
    int rfGain;
    quint32 centerFreq;
    double minFreqScaled;
    double maxFreqScaled;
    quint32 sampleRate;
    int correctionPPM;
    quint32 shiftAmount;
    int samplingMode;
    bool rtlAgc;
    bool offsetTuning;
    bool tunerAgc;
    bool biasTee;
    bool shift;
    bool correctIQ;
    bool invertSpectrum;
    quint32 freqScale;
} device_settings_t;

typedef struct radio_settings {
    int demodMode;
    quint32 bandwidth;
    bool squelch;
    quint32 squelchAmount;
    quint32 cwShiftAmount;
    bool binaural;
    bool antiFading;
    bool lockCarrier;
    bool stickyLock;
} radio_settings_t;

class SdrBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

    Q_PROPERTY(QVariantList sourceOptions READ sourceOptions CONSTANT)
    Q_PROPERTY(QVariantList deviceList READ deviceList NOTIFY deviceListChanged)

    Q_PROPERTY(quint32 centerFreq READ centerFreq WRITE setCenterFreq NOTIFY centerFreqChanged)

    Q_PROPERTY(double minFreqScaled READ minFreqScaled NOTIFY frequencyRangeChanged)
    Q_PROPERTY(double maxFreqScaled READ maxFreqScaled NOTIFY frequencyRangeChanged)
    Q_PROPERTY(QString freqScaleLabel READ freqScaleLabel NOTIFY frequencyRangeChanged)

    Q_PROPERTY(int rfGain READ rfGain WRITE setRfGain NOTIFY rfGainChanged)
    Q_PROPERTY(QVariantList sampleRateOptions READ sampleRateOptions CONSTANT)
    Q_PROPERTY(quint32 sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(int correctionPPM READ correctionPPM WRITE setCorrectionPPM NOTIFY correctionPPMChanged)
    Q_PROPERTY(qint32 shiftAmount READ shiftAmount WRITE setShiftAmount NOTIFY shiftAmountChanged)
    Q_PROPERTY(QVariantList sampleModeOptions READ sampleModeOptions CONSTANT)
    Q_PROPERTY(bool rtlagc READ rtlagc WRITE setRtlAGC NOTIFY rtlagcChanged)
    Q_PROPERTY(bool offsetTuning READ offsetTuning WRITE setOffsetTuning NOTIFY offsetTuningChanged)
    Q_PROPERTY(bool tunerAGC READ tunerAGC WRITE setTunerAGC NOTIFY tunerAGCChanged)
    Q_PROPERTY(bool biasTee READ biasTee WRITE setBiasTee NOTIFY biasTeeChanged)
    Q_PROPERTY(bool shift READ shift WRITE setShift NOTIFY shiftChanged)
    Q_PROPERTY(bool correctIQ READ correctIQ WRITE setCorrectIQ NOTIFY correctIQChanged)
    Q_PROPERTY(bool invertSpectrum READ invertSpectrum WRITE setInvertSpectrum NOTIFY invertSpectrumChanged)
    Q_PROPERTY(int fft_bins READ fft_bins CONSTANT)
    Q_PROPERTY(QVariantList demodModeOptions READ demodModeOptions NOTIFY demodModeChanged)
    Q_PROPERTY(quint32 bandwidth READ bandwidth WRITE setBandwidth NOTIFY bandwidthChanged)
    Q_PROPERTY(bool squelch READ squelch WRITE setSquelch NOTIFY squelchChanged)
    Q_PROPERTY(quint32 squelchAmount READ squelchAmount WRITE setSquelchAmount NOTIFY squelchAmountChanged)
    Q_PROPERTY(quint32 cwShiftAmount READ cwShiftAmount WRITE setCWShiftAmount NOTIFY cwShiftAmountChanged)
    Q_PROPERTY(bool binaural READ binaural WRITE setBinaural NOTIFY binauralChanged)
    Q_PROPERTY(bool antiFading READ antiFading WRITE setAntiFading NOTIFY antiFadingChanged)
    Q_PROPERTY(bool lockCarrier READ lockCarrier WRITE setLockCarrier NOTIFY lockCarrierChanged)
    Q_PROPERTY(bool stickyLock READ stickyLock WRITE setStickyLock NOTIFY stickyLockChanged)
    Q_PROPERTY(quint32 freqScale READ freqScale NOTIFY freqScaleChanged)
public:
    QVariantList sourceOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "RTL-SDR USB"},      {"value", RTL_SDR_USB}}
        };

        return options;
    }

    QVariantList sampleRateOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "3.36 MSPS"},      {"value", Constants::SampleRate::RATE_3_36_MSPS}},
            QVariantMap{{"text", "2.88 MSPS"},      {"value", Constants::SampleRate::RATE_2_88_MSPS}},
            QVariantMap{{"text", "2.4 MSPS"},       {"value", Constants::SampleRate::RATE_2_4_MSPS}},
            QVariantMap{{"text", "1.92 MSPS"},      {"value", Constants::SampleRate::RATE_1_92_MSPS}},
            QVariantMap{{"text", "1.44 MSPS"},      {"value", Constants::SampleRate::RATE_1_44_MSPS}},
            QVariantMap{{"text", "1.2 MSPS"},       {"value", Constants::SampleRate::RATE_1_2_MSPS}},
            QVariantMap{{"text", "0.96 MSPS"},      {"value", Constants::SampleRate::RATE_0_96_MSPS}},
        };
        return options;
    }

    QVariantList sampleModeOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "Quadrature Sampling"},        {"value", 0}},
            QVariantMap{{"text", "Direct Sampling (I Branch)"}, {"value", 1}},
            QVariantMap{{"text", "Direct Sampling (Q Branch)"}, {"value", 2}},
            };

        return options;
    }

    QVariantList demodModeOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "NFM"},    {"value", 0}},
            QVariantMap{{"text", "AM"},     {"value", 1}},
            QVariantMap{{"text", "LSB"},    {"value", 2}},
            QVariantMap{{"text", "USB"},    {"value", 3}},
            QVariantMap{{"text", "WFM"},    {"value", 4}},
            QVariantMap{{"text", "DSB"},    {"value", 5}},
            QVariantMap{{"text", "CW"},     {"value", 6}},
            QVariantMap{{"text", "RAW"},    {"value", 7}},
        };

        return options;
    }

    explicit SdrBackend(QObject *parent = nullptr) : QObject(parent) {
        _chunkQueue = new ChunkQueue(20);
        _latestChunk = new LatestChunk();
        _decimatedChunkQueue = new ChunkQueue(20);

        _rtlReaderThread = new RtlReaderThread(_chunkQueue, _latestChunk, this);
        _rtlAudioThread = new RtlAudioThread(_chunkQueue, _decimatedChunkQueue, this);
        _rtlFFTThread = new RtlFFTThread(_latestChunk, _decimatedChunkQueue, this);

        connect(_rtlFFTThread, &RtlFFTThread::spectrumReady, this, &SdrBackend::spectrum);
        connect(_rtlFFTThread, &RtlFFTThread::narrowSpectrumReady, this, &SdrBackend::narrowSpectrum);
        connect(_rtlFFTThread, &RtlFFTThread::power, this, &SdrBackend::power);

        set_defaults();
    }

    ~SdrBackend() override {
        // Stop producer
        if (_rtlReaderThread) {
            _rtlReaderThread->stop();
            _rtlReaderThread->wait();
        }

        // TODO: should add close to latestchunk?
        _chunkQueue->close(); // close queue and wake the audio thread

        // Stop consumer
        if (_rtlAudioThread) {
            _rtlAudioThread->stop();
            _rtlAudioThread->wait();
        }

        if (_rtlFFTThread) {
            _rtlFFTThread->stop();
            _rtlFFTThread->wait();
        }
    }

    bool running() const { return _running; }

    int source() const { return _device_settings.source; }

    QVariantList deviceList() const { return _device_settings.deviceList; }

    int rfGain() const { return _device_settings.rfGain; }

    quint32 centerFreq() const { return _device_settings.centerFreq; }

    double minFreqScaled() { return _device_settings.minFreqScaled; }

    double maxFreqScaled() { return _device_settings.maxFreqScaled; }

    QString freqScaleLabel() const { return _device_settings.freqScaleLabel; }

    quint32 sampleRate() const { return _device_settings.sampleRate; }

    int correctionPPM() const { return _device_settings.correctionPPM; }

    qint32 shiftAmount() const { return _device_settings.shiftAmount; }

    int samplingMode() const { return _device_settings.samplingMode; }

    bool rtlagc() const { return _device_settings.rtlAgc; }

    bool offsetTuning() const { return _device_settings.offsetTuning; }

    bool tunerAGC() const { return _device_settings.tunerAgc; }

    bool biasTee() const { return _device_settings.biasTee; }

    bool shift() const { return _device_settings.shift; }

    bool correctIQ() const { return _device_settings.correctIQ; }

    bool invertSpectrum() const { return _device_settings.invertSpectrum; }

    int demodMode() const { return _radio_settings.demodMode; }

    int fft_bins() const { return RtlFFTThread::FFT_BINS; }

    quint32 bandwidth() const { return _radio_settings.bandwidth; }

    bool squelch() const { return _radio_settings.squelch; }

    quint32 squelchAmount() const { return _radio_settings.squelchAmount; }

    quint32 cwShiftAmount() const { return _radio_settings.cwShiftAmount; }

    bool binaural() const { return _radio_settings.binaural; }

    bool antiFading() const { return _radio_settings.antiFading; }

    bool lockCarrier() const { return _radio_settings.lockCarrier; }

    bool stickyLock() const { return _radio_settings.stickyLock; }

    quint32 freqScale() const { return _device_settings.freqScale; }

    Q_INVOKABLE void setSource(int source);

    Q_INVOKABLE void setDevice(int deviceIndex);

    Q_INVOKABLE void refreshDevices();

    Q_INVOKABLE void setRfGain(int gain);

    Q_INVOKABLE void setCenterFreq(quint32 freq);

    Q_INVOKABLE void setSampleRate(quint32 sampleRate);

    Q_INVOKABLE void setCorrectionPPM(int correctionPPM);

    Q_INVOKABLE void setShiftAmount(qint32 shiftAmount);

    Q_INVOKABLE void setSamplingMode(int samplingMode);

    Q_INVOKABLE void setRtlAGC(bool rtlagc);

    Q_INVOKABLE void setOffsetTuning(bool offsetTuning);

    Q_INVOKABLE void setTunerAGC(bool tunerAGC);

    Q_INVOKABLE void setBiasTee(bool biasTee);

    Q_INVOKABLE void setShift(bool shift);

    Q_INVOKABLE void setCorrectIQ(bool correctIQ);

    Q_INVOKABLE void setInvertSpectrum(bool invertSpectrum);

    Q_INVOKABLE void setDemodMode(int mode);

    Q_INVOKABLE void setBandwidth(quint32 bandwidth);

    Q_INVOKABLE void setSquelch(bool squelch);

    Q_INVOKABLE void setSquelchAmount(quint32 squelchAmount);

    Q_INVOKABLE void setCWShiftAmount(quint32 cwShiftAmount);

    Q_INVOKABLE void setBinaural(bool binaural);

    Q_INVOKABLE void setAntiFading(bool antiFading);

    Q_INVOKABLE void setLockCarrier(bool lockCarrier);

    Q_INVOKABLE void setStickyLock(bool stickyLock);

    Q_INVOKABLE void setOffsetFreq(double offset);

    Q_INVOKABLE void start();

    Q_INVOKABLE void stop();
signals:
    void runningChanged();
    void deviceListChanged();
    void centerFreqChanged();
    void frequencyRangeChanged();
    void rfGainChanged();
    void correctionPPMChanged();
    void shiftAmountChanged();
    void samplingModeChanged();
    void rtlagcChanged();
    void offsetTuningChanged();
    void tunerAGCChanged();
    void biasTeeChanged();
    void shiftChanged();
    void correctIQChanged();
    void invertSpectrumChanged();
    void errorOccurred(QString message);
    void spectrum(QList<double> bins);
    void narrowSpectrum(QList<double> bins);
    void power(double power);
    void demodModeChanged();
    void bandwidthChanged();
    void sampleRateChanged();
    void squelchChanged();
    void squelchAmountChanged();
    void cwShiftAmountChanged();
    void binauralChanged();
    void antiFadingChanged();
    void lockCarrierChanged();
    void stickyLockChanged();
    void freqScaleChanged();
private:
    ChunkQueue* _chunkQueue = nullptr;
    LatestChunk* _latestChunk = nullptr;
    ChunkQueue* _decimatedChunkQueue = nullptr;

    RtlReaderThread* _rtlReaderThread = nullptr;
    RtlAudioThread* _rtlAudioThread = nullptr;
    RtlFFTThread* _rtlFFTThread = nullptr;

    bool _running = false;
    bool _threadsStart = false;

    const std::vector<frequency_scale_unit_t> frequency_scale_units = {
        { 1, "Hz" },
        { 1e3, "kHz" },
        { 1e6, "MHz" },
        { 1e9, "GHz" }
    };

    device_settings_t _device_settings;

    radio_settings_t _radio_settings;

    void refresh_devices();

    void update_frequency_range();

    int pick_scale_index(uint32_t freq);

    double apply_scale(uint32_t freq, int scaleIndex);

    void set_defaults();

    sdr_config_t create_sdr_config();
};

#endif // SDR_BACKEND_H
