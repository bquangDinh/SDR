#ifndef RTL_AUDIO_THREAD_H
#define RTL_AUDIO_THREAD_H

#include <QThread>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>

#include "core/chunkqueue.h"
#include "core/mixer.h"
#include "core/audiooutput.h"
#include "core/demodulators/demodulator.h"
#include "core/Constants.h"

class RtlAudioThread : public QThread
{
public:
    static constexpr uint32_t AUDIO_SAMPLE_RATE = 48e3;

    RtlAudioThread(ChunkQueue* q, ChunkQueue* decimatedQueue, QObject* parent);
    
    ~RtlAudioThread();

    void pause();

    void stop();

    void resume();
protected:
    void run() override;

private:
    bool _paused = false;
    bool _stop = false;

    uint64_t _offsetFreqSubId,
             _cwShiftAmountSubId,
             _squelchAmountSubId,
             _squelchSubId,
             _bandwidthSubId,
             _demodModeSubId,
             _sampleRateSubId,
             _volumeSubId;

    ChunkQueue* _q = nullptr;
    ChunkQueue* _decimatedQueue = nullptr;

    std::mutex _m;
    std::condition_variable _cv;

    Mixer _mixer;
    AudioOutput _audioOutput;

    std::atomic<int> _requestMode{Constants::DemodulateMode::NFM};
    int _activeMode = -1;
    std::unique_ptr<Demodulator> _demod = nullptr;

    std::vector<std::complex<float>> _shifted;
    std::vector<float> _audio;

    std::unique_ptr<Demodulator> make_demodulator(int m);

    void read();

    void on_offset_freq_changed(const double& offset);
    void on_cw_shift_amount_changed(const uint32_t& cwShiftAmount);
    void on_squelch_amount_changed(const uint32_t& squelchAmount);
    void on_squelch_changed(const bool& squelch);
    void on_volume_changed(const int& volume);
    void on_bandwidth_changed(const uint32_t& bandwidth);
    void on_demod_mode_changed(const int& demodMode);
    void on_sample_rate_changed(const uint32_t& sampleRate);
};

#endif // RTL_AUDIO_THREAD_H
