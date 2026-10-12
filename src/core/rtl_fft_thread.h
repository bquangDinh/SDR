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
    static constexpr uint32_t FFT_BINS = 8192;
    static constexpr uint32_t NARROW_FFT_BINS = 2048;

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
    fftwf_complex* in;
    fftwf_complex* out;
    float _norm = 0;

    void make_blackman_window();
};

#endif // RTL_FFT_THREAD_H
