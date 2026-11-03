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
#include "core/sdr_settings_manager.h"

struct frequency_scale_unit {
    double scale;
    std::string suffix;
};

struct device_settings {
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
};

struct radio_settings {
    int demodMode;
    quint32 bandwidth;
    bool squelch;
    quint32 squelchAmount;
    quint32 cwShiftAmount;
    bool binaural;
    bool antiFading;
    bool lockCarrier;
    bool stickyLock;
};

struct general_settings {
    int volume;
};

class SdrBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QVariantList sourceOptions READ sourceOptions CONSTANT)
    Q_PROPERTY(QVariantList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(quint32 centerFreq READ centerFreq WRITE set_center_freq NOTIFY centerFreqChanged)
    Q_PROPERTY(quint32 offsetFreq READ offsetFreq WRITE set_offset_freq NOTIFY offsetFreqChanged)
    Q_PROPERTY(double minFreqScaled READ minFreqScaled NOTIFY frequencyRangeChanged)
    Q_PROPERTY(double maxFreqScaled READ maxFreqScaled NOTIFY frequencyRangeChanged)
    Q_PROPERTY(QString freqScaleLabel READ freqScaleLabel NOTIFY frequencyRangeChanged)
    Q_PROPERTY(int rfGain READ rfGain WRITE set_rf_gain NOTIFY rfGainChanged)
    Q_PROPERTY(QVariantList sampleRateOptions READ sampleRateOptions CONSTANT)
    Q_PROPERTY(quint32 sampleRate READ sampleRate WRITE set_sample_rate NOTIFY sampleRateChanged)
    Q_PROPERTY(int correctionPPM READ correctionPPM WRITE set_correction_ppm NOTIFY correctionPPMChanged)
    Q_PROPERTY(qint32 shiftAmount READ shiftAmount WRITE set_shift_amount NOTIFY shiftAmountChanged)
    Q_PROPERTY(QVariantList sampleModeOptions READ sampleModeOptions CONSTANT)
    Q_PROPERTY(bool rtlagc READ rtlagc WRITE set_rtl_agc NOTIFY rtlagcChanged)
    Q_PROPERTY(bool offsetTuning READ offsetTuning WRITE set_offset_tuning NOTIFY offsetTuningChanged)
    Q_PROPERTY(bool tunerAGC READ tunerAGC WRITE set_tuner_agc NOTIFY tunerAGCChanged)
    Q_PROPERTY(bool biasTee READ biasTee WRITE set_bias_tee NOTIFY biasTeeChanged)
    Q_PROPERTY(bool shift READ shift WRITE set_shift NOTIFY shiftChanged)
    Q_PROPERTY(bool correctIQ READ correctIQ WRITE set_correct_iq NOTIFY correctIQChanged)
    Q_PROPERTY(bool invertSpectrum READ invertSpectrum WRITE set_invert_spectrum NOTIFY invertSpectrumChanged)
    Q_PROPERTY(int fft_bins READ fft_bins CONSTANT)
    Q_PROPERTY(QVariantList demodModeOptions READ demodModeOptions NOTIFY demodModeChanged)
    Q_PROPERTY(quint32 bandwidth READ bandwidth WRITE set_bandwidth NOTIFY bandwidthChanged)
    Q_PROPERTY(bool squelch READ squelch WRITE set_squelch NOTIFY squelchChanged)
    Q_PROPERTY(quint32 squelchAmount READ squelchAmount WRITE set_squelch_amount NOTIFY squelchAmountChanged)
    Q_PROPERTY(quint32 cwShiftAmount READ cwShiftAmount WRITE set_cw_shift_amount NOTIFY cwShiftAmountChanged)
    Q_PROPERTY(bool binaural READ binaural WRITE set_binaural NOTIFY binauralChanged)
    Q_PROPERTY(bool antiFading READ antiFading WRITE set_anti_fading NOTIFY antiFadingChanged)
    Q_PROPERTY(bool lockCarrier READ lockCarrier WRITE set_lock_carrier NOTIFY lockCarrierChanged)
    Q_PROPERTY(bool stickyLock READ stickyLock WRITE set_sticky_lock NOTIFY stickyLockChanged)
    Q_PROPERTY(quint32 freqScale READ freqScale NOTIFY freqScaleChanged)
    Q_PROPERTY(int volume READ volume WRITE set_volume NOTIFY volumeChanged)
public:
    QVariantList sourceOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "RTL-SDR USB"},      {"value", static_cast<int>(DEVICE_SOURCE::RTL_SDR_USB)}}
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
            QVariantMap{{"text", "Quadrature Sampling"},        {"value", Constants::SamplingMode::QUADRATURE_SAMPLING}},
            QVariantMap{{"text", "Direct Sampling (I Branch)"}, {"value", Constants::SamplingMode::DIRECT_SAMPLING_I_BRANCH}},
            QVariantMap{{"text", "Direct Sampling (Q Branch)"}, {"value", Constants::SamplingMode::DIRECT_SAMPLING_Q_BRANCH}},
            };

        return options;
    }

    QVariantList demodModeOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "NFM"},    {"value", Constants::DemodulateMode::NFM}},
            QVariantMap{{"text", "AM"},     {"value", Constants::DemodulateMode::AM}},
            QVariantMap{{"text", "LSB"},    {"value", Constants::DemodulateMode::LSB}},
            QVariantMap{{"text", "USB"},    {"value", Constants::DemodulateMode::USB}},
            QVariantMap{{"text", "WFM"},    {"value", Constants::DemodulateMode::WFM}},
            QVariantMap{{"text", "DSB"},    {"value", Constants::DemodulateMode::DSB}},
            QVariantMap{{"text", "CW"},     {"value", Constants::DemodulateMode::CW}},
            QVariantMap{{"text", "RAW"},    {"value", Constants::DemodulateMode::RAW}},
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

        init();
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

    int source() const { return _sdr_settings_manager.source(); }

    QVariantList deviceList() const { return _deviceList; }

    int rfGain() const { return _sdr_settings_manager.rfGain(); }

    quint32 centerFreq() const { return _sdr_settings_manager.centerFreq(); }

    double offsetFreq() const { return _sdr_settings_manager.offsetFreq(); }

    double minFreqScaled() { return _minFreqScaled; }

    double maxFreqScaled() { return _maxFreqScaled; }

    QString freqScaleLabel() const { return _freqScaleLabel; }

    quint32 sampleRate() const { return _sdr_settings_manager.sampleRate(); }

    int correctionPPM() const { return _sdr_settings_manager.correctionPPM(); }

    qint32 shiftAmount() const { return _sdr_settings_manager.shiftAmount(); }

    int samplingMode() const { return _sdr_settings_manager.samplingMode(); }

    bool rtlagc() const { return _sdr_settings_manager.rtlAgc(); }

    bool offsetTuning() const { return _sdr_settings_manager.offsetTuning(); }

    bool tunerAGC() const { return _sdr_settings_manager.tunerAgc(); }

    bool biasTee() const { return _sdr_settings_manager.biasTee(); }

    bool shift() const { return _sdr_settings_manager.shift(); }

    bool correctIQ() const { return _sdr_settings_manager.correctIQ(); }

    bool invertSpectrum() const { return _sdr_settings_manager.invertSpectrum(); }

    int demodMode() const { return _sdr_settings_manager.demodMode(); }

    int fft_bins() const { return RtlFFTThread::FFT_BINS; }

    quint32 bandwidth() const { return _sdr_settings_manager.bandwidth(); }

    bool squelch() const { return _sdr_settings_manager.squelch(); }

    quint32 squelchAmount() const { return _sdr_settings_manager.squelchAmount(); }

    quint32 cwShiftAmount() const { return _sdr_settings_manager.cwShiftAmount(); }

    bool binaural() const { return _sdr_settings_manager.binaural(); }

    bool antiFading() const { return _sdr_settings_manager.antiFading(); }

    bool lockCarrier() const { return _sdr_settings_manager.lockCarrier(); }

    bool stickyLock() const { return _sdr_settings_manager.stickyLock(); }

    quint32 freqScale() const { return _freqScale; }

    int volume() const { return _sdr_settings_manager.volume(); }

    Q_INVOKABLE void set_source(int source);

    Q_INVOKABLE void set_device(int deviceIndex);

    Q_INVOKABLE void refresh_devices();

    Q_INVOKABLE void set_rf_gain(int gain);

    Q_INVOKABLE void set_center_freq(quint32 freq);

    Q_INVOKABLE void set_sample_rate(quint32 sampleRate);

    Q_INVOKABLE void set_correction_ppm(int correctionPPM);

    Q_INVOKABLE void set_shift_amount(qint32 shiftAmount);

    Q_INVOKABLE void set_sampling_mode(int samplingMode);

    Q_INVOKABLE void set_rtl_agc(bool rtlagc);

    Q_INVOKABLE void set_offset_tuning(bool offsetTuning);

    Q_INVOKABLE void set_tuner_agc(bool tunerAGC);

    Q_INVOKABLE void set_bias_tee(bool biasTee);

    Q_INVOKABLE void set_shift(bool shift);

    Q_INVOKABLE void set_correct_iq(bool correctIQ);

    Q_INVOKABLE void set_invert_spectrum(bool invertSpectrum);

    Q_INVOKABLE void set_demod_mode(int mode);

    Q_INVOKABLE void set_bandwidth(quint32 bandwidth);

    Q_INVOKABLE void set_squelch(bool squelch);

    Q_INVOKABLE void set_squelch_amount(quint32 squelchAmount);

    Q_INVOKABLE void set_cw_shift_amount(quint32 cwShiftAmount);

    Q_INVOKABLE void set_binaural(bool binaural);

    Q_INVOKABLE void set_anti_fading(bool antiFading);

    Q_INVOKABLE void set_lock_carrier(bool lockCarrier);

    Q_INVOKABLE void set_sticky_lock(bool stickyLock);

    Q_INVOKABLE void set_offset_freq(double offset);

    Q_INVOKABLE void set_volume(int volume);

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
    void volumeChanged();
    void offsetFreqChanged();
private:
    ChunkQueue* _chunkQueue = nullptr;
    LatestChunk* _latestChunk = nullptr;
    ChunkQueue* _decimatedChunkQueue = nullptr;

    RtlReaderThread* _rtlReaderThread = nullptr;
    RtlAudioThread* _rtlAudioThread = nullptr;
    RtlFFTThread* _rtlFFTThread = nullptr;

    bool _running = false;
    bool _threadsStart = false;

    const std::vector<frequency_scale_unit> frequency_scale_units = {
        { 1, "Hz" },
        { 1e3, "kHz" },
        { 1e6, "MHz" },
        { 1e9, "GHz" }
    };

    SdrSettingsManager& _sdr_settings_manager = SdrSettingsManager::instance();

    QVariantList _deviceList;

    QString _freqScaleLabel;

    double _minFreqScaled, _maxFreqScaled;

    quint32 _freqScale;

    void init();

    void update_frequency_range();

    int pick_scale_index(uint32_t freq);

    double apply_scale(uint32_t freq, int scaleIndex);

    sdr_config create_sdr_config();
};

#endif // SDR_BACKEND_H
