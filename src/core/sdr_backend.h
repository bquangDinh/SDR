#ifndef SDR_BACKEND_H
#define SDR_BACKEND_H

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QDebug>

#include "core/rtl_thread.h"

class SdrBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(quint32 centerFreq READ centerFreq WRITE setCenterFreq NOTIFY centerFreqChanged)
    Q_PROPERTY(int rfGain READ rfGain WRITE setRfGain NOTIFY rfGainChanged)
public:
    explicit SdrBackend(QObject *parent = nullptr) : QObject(parent) {}

    ~SdrBackend() override {
        if (rtl_) {
            rtl_->requestInterruption();
            rtl_->wait(); // join thread
        }
    }

    bool running() const { return rtl_ != nullptr; }

    int rfGain() const { return rfGain_; }

    quint32 centerFreq() const { return centerFreq_; }

    Q_INVOKABLE void start() {
        if (rtl_) return;

        rtl_ = new RtlThread(this);

        rtl_->setCenterFreq(centerFreq());

        rtl_->setRfGain(rfGain());

        connect(rtl_, &RtlThread::spectrumReady, this, &SdrBackend::spectrum);
        connect(rtl_, &RtlThread::error, this, &SdrBackend::errorOccurred);
        connect(rtl_, &QThread::finished, this, [this] {
            rtl_->deleteLater();
            rtl_ = nullptr;
            emit runningChanged();
        });

        rtl_->start();

        emit runningChanged();
    }

    Q_INVOKABLE void stop() {
        if (rtl_) rtl_->requestInterruption();
    }

    Q_INVOKABLE void setRfGain(int gain) {
        if (rfGain_ == gain) return;

        if (rtl_ != nullptr) {
            rtl_->setRfGain(gain);
        }

        rfGain_ = gain;

        emit rfGainChanged();
    }

    Q_INVOKABLE void setCenterFreq(quint32 freq) {
        if (this->centerFreq_ == freq) return;

        if (rtl_ != nullptr) {
            rtl_->setCenterFreq(freq);
        }

        this->centerFreq_ = freq;

        emit centerFreqChanged();
    }

signals:
    void runningChanged();
    void centerFreqChanged();
    void rfGainChanged();

    void errorOccurred(QString message);
    void spectrum(QList<double> bins);

private:
    RtlThread* rtl_ = nullptr;

    // Configuration
    int rfGain_ = 0;
    quint32 centerFreq_ = 162e6;
};

#endif // SDR_BACKEND_H
