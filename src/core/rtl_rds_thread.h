#ifndef RTL_RDS_H
#define RTL_RDS_H

#include <QThread>
#include <string>
#include <chrono>

#include "core/chunkqueue.h"
#include "core/fm_discriminator.h"
#include "core/fast_decimator.h"
#include "core/mixer.h"
#include "core/spectrum.h"

class Resampler {
public:
    Resampler(uint32_t up, uint32_t down, uint32_t fs);

    void process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);

    void process_optimal(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
private:
    uint32_t _upRate;
    uint32_t _downRate;
    uint32_t _fs;
    uint32_t _half_len;
    
    size_t _k = 0;
    size_t _H = 0;

    std::vector<std::complex<float>> _hist;

    // The next position of downsample
    // Since data arrives in chunk, the next position that needs to be picked
    // could be some offset away from the start of the current chunk.
    size_t _bufPos = 0;
    
    std::vector<float> _taps;
};

class CFC {
    // Coarse Frequency Correction
private:
    // Minimum buffer size for processing (in samples)
    static constexpr size_t MIN_BUF_SIZE = 1024;

    // FFT size for spectrum analysis
    static constexpr size_t FFT_SIZE = 1024;

    uint32_t _fs;
    Spectrum _spectrum;
    Mixer _mixer;
    float _lastFreq = 0.0f;

    std::vector<std::complex<float>> _buf;
public:
    CFC(uint32_t fs) : _fs(fs), _spectrum(CFC::FFT_SIZE), _mixer() {}

    void process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
};

class AGC {
private:
    float _target, _attack, _release, _maxGain, _gain, _rms;
public:
    AGC(float target, float attack, float release, float maxGain)
        : _target(target), _attack(attack), _release(release), _maxGain(maxGain), _gain(1.0f), _rms(0.0f) {}

    void process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
};

class BPSKDemod {
private:
    uint8_t _lastBit = 0;
    bool _hasLastBit = false;

public:
    void process(const std::complex<float>* data, size_t len, std::vector<uint8_t>& out);
};

enum class SymbolSyncMethod {
    ZeroCrossing,
    Mueller,
    Gardner
};

class SymbolSync {
private:
    int _sps;
    float _gain;
    int _prev_decision;
    float _pos;
    std::vector<std::complex<float>> _prev_samples;
    std::complex<float> _prev_symbol{0.0f, 0.0f};

private:
    void zero_crossing(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
    void mueller(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
    void gardner(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
public:
    SymbolSync(int sps, float gain);

    void process(const std::complex<float>* data, size_t len, SymbolSyncMethod method, std::vector<std::complex<float>>& out);
};

class CostaLoop {
private:
    float _alpha, _beta;
    double _phase, _freq;

public:
    CostaLoop(float alpha, float beta);

    void process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out);
};

class RDSDecoder {
private:
    static constexpr uint16_t OFFSET_WORDS[5] = {0x0FC, 0x198, 0x168, 0x1B4, 0x350};
    static constexpr uint16_t SYNDROME[5] = {383, 14, 303, 663, 748};
    static constexpr uint8_t OFFSET_POS[5] = {0, 1, 2, 3, 2};

    uint32_t reg = 0;
    int bitCount = 0;
    bool synced = false;
    bool preSynced = false;
    int lastseenOffset = -1;
    int lastseenOffsetCounter = -1;
    int wrongBlksCounter = 0;
    int blockBitCounter = 0;
    int blockNumber = 0;
    bool groupAssemblyStarted = false;
    int blocksCounter = 0;
    int groupGoodBlocksCounter = 0;
    int blockDistance = 0;
    uint64_t group = 0;

    uint32_t crc(uint32_t word, size_t len);

public:
    void process(const uint8_t* data, size_t len, std::vector<uint64_t>& out);
};

class RDSParser {
private:
    static constexpr const char* PTY_TABLES[32][2] = {
            {"Undefined",             "Undefined"},
            {"News",                  "News"},
            {"Current Affairs",       "Information"},
            {"Information",           "Sports"},
            {"Sport",                 "Talk"},
            {"Education",             "Rock"},
            {"Drama",                 "Classic Rock"},
            {"Culture",               "Adult Hits"},
            {"Science",               "Soft Rock"},
            {"Varied",                "Top 40"},
            {"Pop Music",             "Country"},
            {"Rock Music",            "Oldies"},
            {"Easy Listening",        "Soft"},
            {"Light Classical",       "Nostalgia"},
            {"Serious Classical",     "Jazz"},
            {"Other Music",           "Classical"},
            {"Weather",               "Rhythm & Blues"},
            {"Finance",               "Soft Rhythm & Blues"},
            {"Children's Programmes", "Language"},
            {"Social Affairs",        "Religious Music"},
            {"Religion",              "Religious Talk"},
            {"Phone-In",              "Personality"},
            {"Travel",                "Public"},
            {"Leisure",               "College"},
            {"Jazz Music",            "Spanish Talk"},
            {"Country Music",         "Spanish Music"},
            {"National Music",        "Hip Hop"},
            {"Oldies Music",          "Unassigned"},
            {"Folk Music",            "Unassigned"},
            {"Documentary",           "Weather"},
            {"Alarm Test",            "Emergency Test"},
            {"Alarm",                 "Emergency"}
        };

    static constexpr const char* COVERAGE_AREA_CODES[16] = {
        "Local",
        "International",
        "National",
        "Supra-regional",
        "Regional 1",
        "Regional 2",
        "Regional 3",
        "Regional 4",
        "Regional 5",
        "Regional 6",
        "Regional 7",
        "Regional 8",
        "Regional 9",
        "Regional 10",
        "Regional 11",
        "Regional 12"
    };

    static constexpr int PTY_LOCALE = 1; // 0 is Europe, 1 is US

    int radioTextABFlag = 0;
    char radioText[65] = {0};
    int programIdentifier = 0;
    int programType = 0;
    char coverageArea[65] = {0};
    int piProgramRefNumber = 0;
    int groupType = 0;

public:
    void process(const uint64_t* rds_data, size_t len, std::string& outRadioText);
};

class RtlRDSThread : public QThread
{
    Q_OBJECT
public:
    RtlRDSThread(ChunkQueue* q, QObject* parent);

    void pause();

    void stop();

    void resume();

protected:
    void run() override;

signals:
    void radioTextUpdated(const std::string& radioText);

    // Debug purposes
    void IQUpdated(const std::vector<std::complex<float>>& iq);

    void rdsSpectrumUpdated(const QList<double> spectrum);
private:
    bool _paused = false;
    bool _stop = false;

    std::atomic<bool> _dirty = true;

    uint64_t _sampleRateSubId;
    uint64_t _offsetFreqSubId;

    std::unique_ptr<FastDecimator> _generalLowPass, _rdsLowPass, _manchesterLowPass;
    std::unique_ptr<FmDiscriminator> _fm;
    std::unique_ptr<Mixer> _mixer;
    std::unique_ptr<CFC> _cfc;
    std::unique_ptr<BPSKDemod> _bpsk;
    std::unique_ptr<SymbolSync> _symbolSync;
    std::unique_ptr<RDSDecoder> _rdsDecoder;
    std::unique_ptr<RDSParser> _rdsParser;
    std::unique_ptr<CostaLoop> _costaLoop;
    std::unique_ptr<AGC> _agc;
    std::unique_ptr<Mixer> _generalMixer;
    std::unique_ptr<Spectrum> _rdsSpectrum;
    std::unique_ptr<Resampler> _resampler;

    ChunkQueue* _q = nullptr;
    std::mutex _m;
    std::condition_variable _cv;

    std::vector<std::complex<float>> _o1, _o2, _iq;
    std::vector<std::complex<float>> _rdsSpectrumSamples;
    std::vector<float> _o3;
    std::vector<std::complex<float>> _buf;
    std::vector<uint64_t> _rdsData;
    std::vector<uint8_t> _bpskOut;
    std::string _radioText;
    std::string _lastEmittedText;
    std::chrono::steady_clock::time_point _lastEmit = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point _lastEmitRDSSpectrum = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point _lastEmitRadioText = std::chrono::steady_clock::now();

    void init();

    void read();

    void on_sample_rate_changed(const uint32_t& sampleRate);
    void on_offset_freq_changed(const int64_t& offsetFreq);
};

#endif // RTL_RDS_H
