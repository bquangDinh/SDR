#ifndef RTL_AUDIO_THREAD_H
#define RTL_AUDIO_THREAD_H

#include <QThread>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>

#include "core/chunkqueue.h"
#include "core/Constants.h"
#include "core/mixer.h"
#include "core/audiooutput.h"
#include "core/demodulators/demodulator.h"

enum class DemodMode {
    NONE, AM, NFM, WFM
};

class RtlAudioThread : public QThread
{
public:
    RtlAudioThread(ChunkQueue* q, ChunkQueue* decimatedQueue, QObject* parent) :
        QThread(parent),
        _q(q),
        _decimatedQueue(decimatedQueue),
        _mixer(),
        _audioOutput()
    {
        _demod = make_demodulator(DemodMode::NFM);
    }

    void pause();

    void stop();

    void resume();

    void setSampleRate(uint32_t sampleRate);

    void setBandwidth(uint32_t bw);

    void setDemodMode(int mode);

    void setOffsetFreq(double offset);
protected:
    void run() override;

private:
    bool _paused = false;
    bool _stop = false;

    ChunkQueue* _q = nullptr;
    ChunkQueue* _decimatedQueue = nullptr;

    std::mutex _m;
    std::condition_variable _cv;

    Mixer _mixer;
    AudioOutput _audioOutput;

    std::atomic<DemodMode> _requestMode{DemodMode::NFM};
    DemodMode _activeMode = DemodMode::NONE;
    std::unique_ptr<Demodulator> _demod = nullptr;

    std::vector<std::complex<float>> _shifted;
    std::vector<float> _audio;

    uint32_t _sampleRate = Constants::SampleRate::RATE_2_4_MSPS;
    uint32_t _bandwidth = 8000;

    std::unique_ptr<Demodulator> make_demodulator(DemodMode m);

    void read();
};

#endif // RTL_AUDIO_THREAD_H
