#include "audiooutput.h"

#include <QDebug>

AudioOutput::~AudioOutput() {
    if (initialized()) {
        if (started()) audioSink->stop();

        audioSink->deleteLater();

        audioSink = nullptr;

        audioDevice = nullptr;
    }
}

void AudioOutput::start() {
    if (!initialized()) {
        qDebug() << "Audio device is not iniatialized";

        return;
    }

    if (started()) {
        qDebug() << "Audio device has started";

        return;
    }

    audioDevice = audioSink->start();

    _started = true;
}

void AudioOutput::stop() {
    audioSink->stop();

    _started = false;

    audioDevice = nullptr;
}

void AudioOutput::init() {
    QAudioFormat format;

    format.setSampleRate(48000); // 48 kHz
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format);

    audioSink->setBufferSize(9600);

    _init = true;
}

void AudioOutput::reset() {
    pcm.clear();
    audioOutputBuffer.clear();
}

void AudioOutput::write(const std::vector<float>& audio) {
    if (!initialized() || !started() || !audioDevice) {
        qDebug() << "Audio device has not initialized and started!";

        return;
    }

    pcm.resize(audio.size());

    for (size_t i = 0; i < audio.size(); ++i) {
        float clamped = std::clamp(audio[i] * _volume, -1.0f, 1.0f);

        pcm[i] = static_cast<int16_t>(clamped * 32767.0f);
    }

    audioOutputBuffer.insert(audioOutputBuffer.end(), pcm.begin(), pcm.end());

    if (audioOutputBuffer.size() >= FLUSH_THRESHOLD) {
        qint64 bytesToWrite = audioOutputBuffer.size() * sizeof(uint16_t);

        qint64 bytesWritten = audioDevice->write(reinterpret_cast<const char*>(audioOutputBuffer.data()), bytesToWrite);

        if (bytesWritten < bytesToWrite) {
            qDebug() << "AUDIO UNDERRUN: wanted to write" << bytesToWrite << "but only wrote" << bytesWritten;
            // keep the unwritten remainder for next time, instead of dropping it:
            size_t samplesWritten = bytesWritten / sizeof(int16_t);
            audioOutputBuffer.erase(audioOutputBuffer.begin(), audioOutputBuffer.begin() + samplesWritten);
        } else {
            audioOutputBuffer.clear();
        }
    }
}