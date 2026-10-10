#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <complex>
#include <cassert>

class Demodulator
{
public:
    Demodulator() {}

    uint32_t sampleRate() const { return this->_sampleRate; }

    uint32_t audioSampleRate() const { return this->_audioSampleRate; }

    void setSampleRate(uint32_t sampleRate) {
        assert(sampleRate > 0 && "Sample rate must larger than 0");
        assert(sampleRate > _audioSampleRate && "Sample rate must be larger than audio sample rate");
        assert(sampleRate % _audioSampleRate == 0 && "Sample rate must be dividable to audio sample rate");

        this->_sampleRate = sampleRate;

        this->_decimatedStep = sampleRate / _audioSampleRate;
    }

    void setAudioSampleRate(uint32_t audioSampleRate) {
        assert(audioSampleRate > 0 && "Audio sample rate must larger than 0");
        assert(_sampleRate > audioSampleRate && "Sample rate must be larger than audio sample rate");
        assert(_sampleRate % audioSampleRate == 0 && "Sample rate must be dividable to audio sample rate");

        this->_audioSampleRate = audioSampleRate;

        this->_decimatedStep = _sampleRate / audioSampleRate;
    }

    void demodulate_nfm(const std::complex<float>* data, size_t size, std::vector<uint16_t>& out);
private:
    bool _hasLastSample = false;
    std::complex<float> _lastDecimatedSample = 0;

    uint32_t _decimatedStep = 0;
    uint32_t _sampleRate = 0;
    uint32_t _audioSampleRate = 48e3;
};

#endif // DEMODULATOR_H
