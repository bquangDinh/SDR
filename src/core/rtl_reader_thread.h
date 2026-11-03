#ifndef RTL_READER_THREAD_H
#define RTL_READER_THREAD_H

#include <QThread>
#include <condition_variable>
#include <mutex>

#include "core/chunkqueue.h"
#include "core/latestchunk.h"

class RtlReaderThread : public QThread
{
public:
    static constexpr uint32_t BUFFER_SIZE = 8192;

    RtlReaderThread(ChunkQueue* q, LatestChunk* c, QObject* parent);

    ~RtlReaderThread();

    void pause();

    void stop();

    void resume();
protected:
    void run() override;
private:
    ChunkQueue* _queue;
    LatestChunk* _latestChunk;

    uint8_t buf[RtlReaderThread::BUFFER_SIZE];

    std::mutex _m;
    std::condition_variable _cv;
    bool _paused = false;
    bool _stop = false;

    bool _invertSpectrum = false;
    bool _correctIQ = false;

    uint64_t _sampleRateSubId;

    // Running sum averages of I, Q
    // use for Correct IQ
    double _alpha, _muI, _muQ;
    uint32_t _sampleRate;
    const double _correctIQCutOff = 5; // cutoff at 5 Hz

    void read();

    void on_sample_rate_changed(const uint32_t& v);
};

#endif // RTL_READER_THREAD_H
