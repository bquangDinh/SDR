#ifndef CHUNKQUEUE_H
#define CHUNKQUEUE_H

#include <complex>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <deque>

using Chunk = std::shared_ptr<const std::vector<std::complex<float>>>;

class ChunkQueue
{
public:
    explicit ChunkQueue(size_t capacity) : _cap(capacity) {}

    void push(Chunk c);

    Chunk pop(std::chrono::milliseconds timeout);

    Chunk try_pop();

    void close();

    size_t dropped();
private:
    bool _closed = false;
    size_t _cap = 0, _dropped = 0;
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<Chunk> _q;
};

#endif // CHUNKQUEUE_H
