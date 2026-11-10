#ifndef CORE_H
#define CORE_H

#include <complex>
#include <vector>
#include <stdint.h>

namespace Core {
    namespace Synchronizers {
        enum class SymbolSyncMethod {
            ZeroCrossing,
            Gardner,
            MuellerAndMuller,
        };

        class CostasLoop {
            private:
                float _alpha, _beta;
                double _phase = 0, _freq = 0;

            public:
                CostasLoop(float alpha, float beta)
                    : _alpha(alpha), _beta(beta) {}

                void process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);

                void reset();
        };

        class SymbolSync {
            private:
                int _sps, _prev_decision;
                float _gain, _pos;
                SymbolSyncMethod _method;
                std::vector<std::complex<float>> _history;
                std::complex<float> _prev_symbol;

                void zero_crossing(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);
                void gardner(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);
                void mueller_and_muller(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);
            public:
                SymbolSync(int sps, float gain, SymbolSyncMethod method)
                    : _sps(sps), _gain(gain), _method(method), _prev_decision(0), _pos(sps) {}
                
                void process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);

                void reset();

                void set_sps(int sps);

                void set_gain(float gain);

                void set_symbol_sync_method(SymbolSyncMethod method);
        };
    };

    namespace LevelControllers {
        class AGC {
            private:
                float _target, _attack, _release, _maxGain, _gain, _rms;
            
            public:
                AGC(float target, float attack, float release, float maxGain)
                    : _target(target), _attack(attack), _release(release), _maxGain(maxGain), _gain(1.0f), _rms(0.0f) {}

                void process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);

                void reset();

                void set_target(float target);

                void set_attack(float attack);

                void set_release(float release);

                void set_max_gain(float maxGain);
        };
    };

    namespace Resamplers {
        class RationalResampler {
            private:
                uint32_t _up, _down, _fs, _half_len;

                size_t _k = 0, _H = 0, _start = 0;

                std::vector<std::complex<float>> _history;

                std::vector<float> _taps;

                bool _initialized = false;

                void init();
            public:
                RationalResampler(uint32_t up, uint32_t down, uint32_t fs);

                void process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out);

                void reset();

                void set_up(uint32_t up);

                void set_down(uint32_t down);

                void set_fs(uint32_t fs);
        };
    };

    namespace Modulators {
        class BPSK {
            private:
                uint8_t _lastBit = 0;
                bool _hasLastBit = false;

            public:
                void process(const std::complex<float>* in, size_t len, std::vector<uint8_t>& out);

                void reset();
        };
    };

    namespace RDS {
        enum class RDSLocale {
            Europe = 0,
            USA = 1
        };

        struct RDSInfo {
            RDSLocale locale;
            int piProgramRefNumber;
            int groupType;
            char radioText[65];
            char coverageArea[65];
            char pty[65];
        };

        class Decoder {
            private:
                static constexpr uint16_t OFFSET_WORDS[5] = {0x0FC, 0x198, 0x168, 0x1B4, 0x350};
                static constexpr uint16_t SYNDROME[5] = {383, 14, 303, 663, 748};
                static constexpr uint8_t OFFSET_POS[5] = {0, 1, 2, 3, 2};
                static constexpr uint32_t CRC_POLY = 0x5B9;

                uint32_t _reg = 0;
                uint64_t _group = 0;
                int _bitCount = 0, _lastSeenOffset = -1, _lastSeenOffsetCounter = -1;
                int _wrongBlksCounter = 0, _blockBitCounter = 0, _blockNumber = 0;
                int _blocksCounter = 0, _groupGoodBlocksCounter = 0, _blockDistance = 0;
                bool _synced = false, _preSynced = false, _groupAssemblyStarted = false;

                uint32_t crc(uint32_t word, size_t len);

            public:
                void process(const uint8_t* in, size_t len, std::vector<uint64_t>& out);

                void reset();
        };

        class Parser {
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
                
                RDSInfo _rdsInfo;

                int _radioTextABFlag = 0;

                RDSLocale _locale;
            public:
                Parser(RDSLocale locale);

                void process(const uint64_t* rds, size_t len, RDSInfo& info);

                void reset();
        };
    };
};

#endif // CORE_H
