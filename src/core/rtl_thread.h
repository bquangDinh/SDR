#ifndef RTL_THREAD_H
#define RTL_THREAD_H

#include <QThread>
#include <QByteArray>
#include <QDebug>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>

#include <complex>
#include <chrono>

#include "core/sdr_device_manager.h"

#define BUFFER_SIZE 65536

class RtlThread : public QThread {
    Q_OBJECT
public:
    explicit RtlThread(QObject *parent = nullptr) : QThread(parent) {
        QAudioFormat format;

        format.setSampleRate(48000); // 48 kHz
        format.setChannelCount(1);
        format.setSampleFormat(QAudioFormat::Int16);

        audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);

        audioSink->setBufferSize(9600);

        connect(audioSink, &QAudioSink::stateChanged, [](QAudio::State state) {
            qDebug() << "Audio sink state changed to:" << state;
        });
    }

signals:
    void spectrumReady(QList<double> data);
    void error(QString msg);

private:
    QAudioSink *audioSink = nullptr;
    QIODevice *audioDevice = nullptr;

    std::vector<int16_t> _audioOutputBuffer;

    std::complex<float> _lastDecimatedSample = 0;
    bool _hasLastSample = false;

    std::vector<std::complex<float>> _filterHistory;

    void fft(std::vector<std::complex<float>>& x, int N, std::complex<float>* outBuf);
protected:
    void get_iq_stream(const char* buf, int n, std::vector<std::complex<float>>& out);

    void sample_fft(std::vector<std::complex<float>>& iqStream, int binsCount, std::complex<float>* outBuf);

    void sample(std::vector<std::complex<float>>& iqStream, int binsCount, std::complex<float>* outBuf);

    void toDBFS(const std::complex<float>* X, int binsCount, std::vector<float> &out);

    std::vector<float> make_filter(uint32_t fPass, uint32_t fStop, uint32_t fs);

    std::vector<std::complex<float>> filter(std::vector<std::complex<float>>& iqStream, std::vector<float>& taps, int decimate);

    void demodulate(std::vector<std::complex<float>>& iqStream);

    void run() override {
        SdrDevice* currentDevice = SdrDeviceManager::instance().currentDevice();

        if (currentDevice == nullptr) {
            qDebug() << "No device selected";

            emit error("No device is selected");

            return;
        }

        qDebug() << "worker started, thread id: " << QThread::currentThreadId();

        SdrDeviceManager::instance().init_current_device();

        qDebug() << "Initialized current device | freq: " << currentDevice->config().centerFreq;

        std::vector<float> taps = make_filter(8000, 24000, currentDevice->config().sampleRate);

        audioDevice = audioSink->start();

        qDebug() << "Start audio device";

        int read = 0;

        char buf[BUFFER_SIZE];

        std::vector<std::complex<float>> iqStream(BUFFER_SIZE / 2);
        std::complex<float> processed[BUFFER_SIZE / 2];
        std::vector<float> dbfss(BUFFER_SIZE / 2);

        while (!isInterruptionRequested()) {
            read = currentDevice->sdr_read_data_sync((void*)buf, BUFFER_SIZE);

            if (read < 0) {
                emit error("sdr_read failed");

                break;
            }

            if (read == 0) continue;

            get_iq_stream(buf, BUFFER_SIZE, iqStream);

            std::vector<std::complex<float>> filtered = filter(iqStream, taps, 50);

            demodulate(filtered);

            // sample_fft(iqStream, BUFFER_SIZE / 2, processed);

            // toDBFS(processed, BUFFER_SIZE / 2, dbfss);

            // emit spectrumReady(QList<double>(dbfss.begin(), dbfss.end()));
        }

        SdrDeviceManager::instance().destroy_current_device();

        qDebug() << "Destroyed current device";

        audioSink->stop();

        audioSink->deleteLater();

        audioSink = nullptr;

        audioDevice = nullptr;

        qDebug() << "Destroyed audio device";
    }
};

#endif // RTL_THREAD_H
