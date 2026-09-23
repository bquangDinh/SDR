#ifndef RTL_THREAD_H
#define RTL_THREAD_H

#include <QThread>
#include <QByteArray>
#include <QDebug>
#include <complex>

#include "core/sdr_device_manager.h"

#define BUFFER_SIZE 2048

class RtlThread : public QThread {
    Q_OBJECT
public:
    explicit RtlThread(QObject *parent = nullptr) : QThread(parent) {}

signals:
    void spectrumReady(QList<double> data);
    void error(QString msg);

protected:
    void sample(const char* buf, int n, int binsCount, std::complex<float>* outBuf);

    void toDBFS(const std::complex<float>* X, int binsCount, std::vector<float> &out);

    void run() override {
        SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

        if (currentDevice == nullptr) {
            qDebug() << "No device selectecd";

            emit error("No device is selected");

            return;
        }

        qDebug() << "worker started, thread id: " << QThread::currentThreadId();

        int read = 0;

        char buf[BUFFER_SIZE];

        std::complex<float> processed[BUFFER_SIZE / 2];
        std::vector<float> dbfss(BUFFER_SIZE / 2);

        while (!isInterruptionRequested()) {
            read = currentDevice->sdr_read_data_sync((void*)buf, BUFFER_SIZE);

            if (read < 0) {
                emit error("sdr_read failed");

                break;
            }

            if (read == 0) continue;

            sample(buf, BUFFER_SIZE, BUFFER_SIZE / 2, processed);

            toDBFS(processed, BUFFER_SIZE / 2, dbfss);

            emit spectrumReady(QList<double>(dbfss.begin(), dbfss.end()));

            msleep(50);
        }
    }
};

#endif // RTL_THREAD_H
