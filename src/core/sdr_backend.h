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
public:
    explicit SdrBackend(QObject *parent = nullptr) : QObject(parent) {}

    ~SdrBackend() override {
        if (rtl_) {
            rtl_->requestInterruption();
            rtl_->wait(); // join thread
        }
    }

    bool running() const { return rtl_ != nullptr; }
    int chunkCount() const { return chunks_; }

    Q_INVOKABLE void start() {
        if (rtl_) return;

        chunks_ = 0;

        rtl_ = new RtlThread(this);

        connect(rtl_, &RtlThread::samples, this, &SdrBackend::onSamples);
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

signals:
    void runningChanged();
    void chunkCountChanged();
    void errorOccurred(QString message);
    void spectrum(QList<double> bins);

private slots:
    void onSamples(QByteArray data) {
        ++chunks_;

        emit chunkCountChanged();

        if (chunks_ % 20 == 0) {
            qDebug() << "UI Thread got chunk " << chunks_ << " size " << data.size() << " thread id: " << QThread::currentThreadId();
        }

        QList<double> bins(64, -80.0);

        emit spectrum(bins);
    }

private:
    RtlThread* rtl_ = nullptr;
    int chunks_ = 0;
};

#endif // SDR_BACKEND_H
