#ifndef LATESTCHUNK_H
#define LATESTCHUNK_H

#include <memory>
#include <mutex>
#include <vector>
#include <complex>

using Chunk = std::shared_ptr<const std::vector<std::complex<float>>>;

class LatestChunk
{
public:
    LatestChunk();

    void put(Chunk c);

    Chunk take();
private:
    std::mutex _m;

    Chunk _latest;
};

#endif // LATESTCHUNK_H
