#ifndef RTL_FFT_THREAD_H
#define RTL_FFT_THREAD_H

#include <QThread>
#include <fftw3.h>

#include "core/latestchunk.h"
#include "core/chunkqueue.h"

class RtlFFTThread : public QThread
{
    Q_OBJECT
public:
    static constexpr uint32_t FFT_BINS = 4096;
    static constexpr uint32_t NARROW_FFT_BINS = 1024;

    RtlFFTThread(LatestChunk* c, ChunkQueue* decimatedQueue, QObject* parent) :
        QThread(parent),
        _c(c),
        _decimatedQueue(decimatedQueue)
    {}

    void pause();

    void stop();

    void resume();
signals:
    void spectrumReady(QList<double> data);
    void narrowSpectrumReady(QList<double> data);
    void power(double power);
protected:
    void run() override;

private:
    LatestChunk* _c = nullptr;
    ChunkQueue* _decimatedQueue = nullptr;

    std::mutex _m;
    std::condition_variable _cv;
    bool _paused = false;
    bool _stop = false;

    std::vector<float> fullSpectrumOutput;
    std::vector<float> narrowSpectrumOutput;

    void make_blackman_window();

    double compute_spectrum_power(const std::complex<float>* data, size_t n);
};

#endif // RTL_FFT_THREAD_H
