#ifndef RTL_THREAD_H
#define RTL_THREAD_H

#include <QThread>
#include <QByteArray>
#include <QDebug>

#include "core/sdr_device.h"

class RtlThread : public QThread {
    Q_OBJECT
public:
    explicit RtlThread(QObject *parent = nullptr) : QThread(parent) {}

signals:
    void samples(QByteArray data);
    void error(QString msg);

protected:
    void run() override {
        qDebug() << "worker started, thread id: " << QThread::currentThreadId();

        if (sdr_init() < 0) {
            emit error("sdr_init failed");

            return;
        }

        int read = 0;

        char buf[1024];

        while (!isInterruptionRequested()) {
            read = sdr_read((void*)buf, 1024);

            if (read < 0) {
                emit error("sdr_read failed");
                break;
            }

            if (read == 0) continue;

            emit samples(QByteArray(buf, 1024));

            msleep(50);
        }

        sdr_destroy();
    }
};

#endif // RTL_THREAD_H
