#ifndef SDR_BACKEND_H
#define SDR_BACKEND_H

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QDebug>
#include <QVariant>

#include "core/rtl_thread.h"
#include "core/Constants.h"
#include "core/sdr_device_manager.h"

typedef struct frequency_scale_unit {
    double scale;
    std::string suffix;
} frequency_scale_unit_t;

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

public:
    QVariantList sourceOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "RTL-SDR USB"},      {"value", RTL_SDR_USB}}
        };

        return options;
    }

    QVariantList sampleRateOptions() const {
        static const QVariantList options = {
            QVariantMap{{"text", "3.2 MSPS"},      {"value", Constants::SampleRate::RATE_3_2_MSPS}},
            QVariantMap{{"text", "2.8 MSPS"},      {"value", Constants::SampleRate::RATE_2_8_MSPS}},
            QVariantMap{{"text", "2.56 MSPS"},     {"value", Constants::SampleRate::RATE_2_56_MSPS}},
            QVariantMap{{"text", "2.4 MSPS"},      {"value", Constants::SampleRate::RATE_2_4_MSPS}},
            QVariantMap{{"text", "2.048 MSPS"},    {"value", Constants::SampleRate::RATE_2_048_MSPS}},
            QVariantMap{{"text", "1.92 MSPS"},     {"value", Constants::SampleRate::RATE_1_92_MSPS}},
            QVariantMap{{"text", "1.8 MSPS"},      {"value", Constants::SampleRate::RATE_1_8_MSPS}},
            QVariantMap{{"text", "1.4 MSPS"},      {"value", Constants::SampleRate::RATE_1_4_MSPS}},
            QVariantMap{{"text", "1.024 MSPS"},    {"value", Constants::SampleRate::RATE_1_024_MSPS}},
            QVariantMap{{"text", "0.900001 MSPS"}, {"value", Constants::SampleRate::RATE_0_900001_MSPS}},
            QVariantMap{{"text", "0.25 MSPS"},     {"value", Constants::SampleRate::RATE_0_25_MSPS}}
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

    explicit SdrBackend(QObject *parent = nullptr) : QObject(parent) {}

    ~SdrBackend() override {
        if (_rtl) {
            _rtl->requestInterruption();
            _rtl->wait(); // join thread
        }
    }

    bool running() const { return _rtl != nullptr; }

    int source() const { return _source; }

    QVariantList deviceList() const { return _deviceList; }

    int rfGain() const { return _rfGain; }

    quint32 centerFreq() const { return _centerFrequency; }

    double minFreqScaled() { return _minFreqScaled; }

    double maxFreqScaled() { return _maxFreqScaled; }

    QString freqScaleLabel() const { return _freqScaleLabel; }

    quint32 sampleRate() const { return _sampleRate; }

    int correctionPPM() const { return _correctionPPM; }

    qint32 shiftAmount() const { return _shiftAmount; }

    int samplingMode() const { return _samplingMode; }

    bool rtlagc() const { return _rtlagc; }

    bool offsetTuning() const { return _offsetTuning; }

    bool tunerAGC() const { return _tunerAGC; }

    bool biasTee() const { return _biasTee; }

    bool shift() const { return _shift; }

    bool correctIQ() const { return _correctIQ; }

    bool invertSpectrum() const { return _invertSpectrum; }

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

private:
    RtlThread* _rtl = nullptr;

    const std::vector<frequency_scale_unit_t> frequency_scale_units = {
        { 1, "Hz" },
        { 1e3, "kHz" },
        { 1e6, "MHz" },
        { 1e9, "GHz" }
    };

    QString _freqScaleLabel;

    // Source Configuration
    int _source = -1; // No source selected
    QVariantList _deviceList;

    int _rfGain = 0;
    quint32 _centerFrequency = 162475000;
    double _minFreqScaled = 0.0;
    double _maxFreqScaled = 0.0;
    quint32 _sampleRate = Constants::SampleRate::RATE_2_4_MSPS;
    int _correctionPPM = 0;
    qint32 _shiftAmount = 0;
    int _samplingMode = 0; // 0 is Quadrature // 1 is I Branch // 2 is Q branch
    bool _rtlagc = false;
    bool _offsetTuning = false;
    bool _tunerAGC = false;
    bool _biasTee = false;
    bool _shift = false;
    bool _correctIQ = false;
    bool _invertSpectrum = false;

    void refresh_devices();

    void update_frequency_range(uint32_t minFreq, uint32_t maxFreq);

    int pick_scale_index(uint32_t freq);

    double apply_scale(uint32_t freq, int scaleIndex);
};

#endif // SDR_BACKEND_H
