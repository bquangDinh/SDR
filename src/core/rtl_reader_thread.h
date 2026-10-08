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
    static constexpr uint32_t BUFFER_SIZE = 65536;

    RtlReaderThread(ChunkQueue* q, LatestChunk* c, QObject* parent) : QThread(parent), _queue(q), _latestChunk(c) {}

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

    void read();
};

#endif // RTL_READER_THREAD_H
