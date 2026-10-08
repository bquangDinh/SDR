#include "latestchunk.h"

#include <utility>

LatestChunk::LatestChunk() {}

void LatestChunk::put(Chunk c) {
    std::lock_guard<std::mutex> lk(_m);

    _latest = std::move(c);
}

Chunk LatestChunk::take() {
    std::lock_guard<std::mutex> lk(_m);

    // Return _latest, and also safely set _latest to nullptr
    return std::exchange(_latest, nullptr);
}
