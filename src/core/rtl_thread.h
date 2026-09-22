#ifndef RTL_THREAD_H
#define RTL_THREAD_H

#include <QThread>
#include <QByteArray>
#include <QDebug>
#include <complex>

#include "core/sdr_device.h"

class RtlThread : public QThread {
    Q_OBJECT
public:
    explicit RtlThread(QObject *parent = nullptr) : QThread(parent) {}

    void setCenterFreq(quint32 freq);
    void setRfGain(int rfGain);

signals:
    void spectrumReady(QList<double> data);
    void error(QString msg);

protected:
    void sample(const char* buf, int n, int binsCount, std::complex<float>* outBuf);
    void toDBFS(const std::complex<float>* X, int binsCount, std::vector<float> &out);

    void run() override {
        qDebug() << "worker started, thread id: " << QThread::currentThreadId();

        if (sdr_init() < 0) {
            emit error("sdr_init failed");

            return;
        }

        sdr_set_center_freq(centerFreq_);

        sdr_set_rf_gain(rfGain_);

        int read = 0;

        char buf[2048];
        std::complex<float> processed[1024];
        std::vector<float> dbfss(1024);

        while (!isInterruptionRequested()) {
            read = sdr_read((void*)buf, 2048);

            if (read < 0) {
                emit error("sdr_read failed");
                break;
            }

            if (read == 0) continue;

            sample(buf, 2048, 1024, processed);

            toDBFS(processed, 1024, dbfss);

            emit spectrumReady(QList<double>(dbfss.begin(), dbfss.end()));

            for (int i = 0; i < 6; i++) qDebug() << processed[i].real() << "+" << processed[i].imag() << "i";

            msleep(50);
        }

        sdr_destroy();
    }

private:
    quint32 centerFreq_ = 162e6;
    int rfGain_ = 0;
};

#endif // RTL_THREAD_H
