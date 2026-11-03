#include "chunkqueue.h"
#include "core/telemetry.h"

void ChunkQueue::push(Chunk c) {
    {
        std::lock_guard<std::mutex> lk(_m);

        if (_q.size() == _cap) {
            // The queue is full, drop the oldest one
            _q.pop_front();

            _dropped++;
        }

        _q.push_back(std::move(c));
    }

    // Notify one waiting thread
    _cv.notify_one();
}

// Try to pop right away
Chunk ChunkQueue::try_pop() {
    Telemetry::instance().start_recording_chunkQueuePopImmediateTime();

    std::lock_guard<std::mutex> lk(_m);

    if (_q.empty()) return nullptr;

    Chunk c = std::move(_q.front());

    _q.pop_front();

    Telemetry::instance().stop_recording_chunkQueuePopImmediateTime();

    return c;
}

Chunk ChunkQueue::pop(std::chrono::milliseconds timeout) {
    Telemetry::instance().start_recording_chunkQueuePopTime(); 
    
    std::unique_lock<std::mutex> lk(_m);

    // Wait if the queue is currently empty or the queue is closed (goes timeout)
    _cv.wait_for(lk, timeout, [&] {
        return !_q.empty() || _closed;
    });

    // Just to make sure, the moment the consumer thread wakes up
    // Another consumer may took the item already
    if (_q.empty()) return nullptr;

    Chunk c = std::move(_q.front());

    _q.pop_front();

    Telemetry::instance().stop_recording_chunkQueuePopTime();

    return c;
}

void ChunkQueue::close() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _closed = true;
    }

    // Notify all waiting threads
    _cv.notify_all();
}

size_t ChunkQueue::dropped() {
    {
        std::lock_guard<std::mutex> lk(_m);

        return _dropped;
    }
}