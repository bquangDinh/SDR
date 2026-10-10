#ifndef RTL_AUDIO_THREAD_H
#define RTL_AUDIO_THREAD_H

#include <QThread>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>

#include "core/chunkqueue.h"
#include "core/fast_decimator.h"
#include "core/Constants.h"

class RtlAudioThread : public QThread
{
public:
    RtlAudioThread(ChunkQueue* q, ChunkQueue* decimatedQueue, QObject* parent) :
        QThread(parent),
        _q(q),
        _decimatedQueue(decimatedQueue)
    {}

    void pause();

    void stop();

    void resume();

    uint32_t bandwidth() const { return _bandwidth; }

    void setSampleRate(uint32_t sampleRate) { this->_sampleRate = sampleRate; }

    void setBandwidth(uint32_t bw) { this->_bandwidth = bw; _init = false; }

    void setDemodMode(int mode) { this->_demod = mode; }

    void setOffsetFreq(double offset);

protected:
    void run() override;

private:
    ChunkQueue* _q = nullptr;
    ChunkQueue* _decimatedQueue = nullptr;

    std::mutex _m;
    std::condition_variable _cv;

    bool _paused = false;
    bool _stop = false;
    bool _init = false;

    QAudioSink *audioSink = nullptr;
    QIODevice *audioDevice = nullptr;
    std::vector<int16_t> _audioOutputBuffer;
    std::complex<float> _lastDecimatedSample = 0;
    std::vector<std::complex<float>> decimated;
    std::vector<std::complex<float>> shifted;
    bool _hasLastSample = false;
    FastDecimator* _dec = nullptr;
    float _carrier = 0.0f;

    uint32_t _sampleRate = Constants::SampleRate::RATE_2_4_MSPS;
    uint32_t _bandwidth = 8000;
    int _demod = Constants::DemodulateMode::NFM;
    double _offsetFreq = 0;

    std::complex<double> _rot{1.0, 0.0};
    std::complex<double> _rotStep{1.0, 0.0};

    // std::vector<float> make_filter(const uint32_t fPass, const uint32_t fStop, const uint32_t sampleRate);

    bool init();

    void shift(const std::complex<float>* data, size_t size, std::vector<std::complex<float>>& out);

    void demodulate();

    void demodulate_am();

    void read();
};

#endif // RTL_AUDIO_THREAD_H
