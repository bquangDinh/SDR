#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>

class AudioOutput
{
public:
    AudioOutput() {}

    ~AudioOutput();

    void init();

    void start();

    void stop();

    void write(const std::vector<float>& audio);

    float volume() const { return this->_volume; }

    bool initialized() const { return this->_init; }

    bool started() const { return this->_started; }

    void set_volume(float volume) { this->_volume = volume; }

    void reset();
private:
    QAudioSink *audioSink = nullptr;
    QIODevice *audioDevice = nullptr;

    bool _init = false;
    bool _started = false;
    float _volume = 1.0f;
    static constexpr size_t FLUSH_THRESHOLD = 960;

    std::vector<uint16_t> pcm;
    std::vector<uint16_t> audioOutputBuffer;
};

#endif // AUDIOOUTPUT_H
