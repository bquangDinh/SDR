#include "core/core.h"
#include "core/signal_utils.h"
#include <algorithm>
#include <string.h>
#include <numeric>

void Core::Synchronizers::CostasLoop::process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    if (len == 0) return;

    out.clear();

    out.resize(len);

    float err = 0.0f;

    for (size_t i = 0; i < len; ++i) {
        // Rotate the input sample "back" by the current phase to correct for carrier frequency offset
        // Without this step, the projection on the real axis will result in bit flip and noise
        out[i] = in[i] * std::exp(std::complex<float>(0, -_phase));

        // Calculate the phase error
        // If the sample lands on the real axis, the imaginary part should be zero ideally, thus err is zero
        // err > 0 indicates it is rotating counter clockwise
        // err < 0 indicates it is rotating clockwise
        err = std::real(out[i]) * std::imag(out[i]);

        // Update the frequency estimate ("spin speed") based on err
        // if err keeps being positive, the frequency estimate will increase
        // if err keeps being negative, the frequency estimate will decrease
        _freq += _beta * err;

        // Compute the phase for the next iteration based on the updated frequency and phase error
        _phase += _freq + _alpha * err;

        // Keep _phase within the range [0, 2*pi) to avoid numerical issues
        if (_phase >= 2 * M_PI) _phase -= 2 * M_PI;
        else if (_phase < 0) _phase += 2 * M_PI;
    }
}

void Core::Synchronizers::CostasLoop::reset() {
    _phase = 0;
    _freq = 0;
}

void Core::Synchronizers::SymbolSync::process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    switch (_method) {
        case SymbolSyncMethod::ZeroCrossing:
            zero_crossing(in, len, out);
            break;
        case SymbolSyncMethod::Gardner:
            gardner(in, len, out);
            break;
        case SymbolSyncMethod::MuellerAndMuller:
            mueller_and_muller(in, len, out);
            break;
    }
}

void Core::Synchronizers::SymbolSync::set_sps(int sps) {
    _sps = sps;
}

void Core::Synchronizers::SymbolSync::set_symbol_sync_method(SymbolSyncMethod method) {
    _method = method;
}

void Core::Synchronizers::SymbolSync::set_gain(float gain) {
    _gain = gain;
}

void Core::Synchronizers::SymbolSync::reset() {
    _prev_decision = 0;
    _pos = _sps;
    _history.clear();
}

void Core::Synchronizers::SymbolSync::zero_crossing(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    if (len == 0) return;

    out.clear();

    size_t total_len = _history.size() + len;

    std::vector<std::complex<float>> buffer(total_len);

    std::copy(_history.begin(), _history.end(), buffer.begin());

    std::copy(in, in + len, buffer.begin() + _history.size());

    // Update the history with the new samples
    const size_t keep = std::min(total_len, static_cast<size_t>(_sps));

    _history.assign(buffer.end() - keep, buffer.end());

    std::complex<float> symbol, mid;
    int decision = 0;
    float err = 0.0f;
    size_t idx, mid_idx;

    while (_pos < total_len) {
        // Round _pos to the nearest integer to get the current symbol index
        idx = static_cast<size_t>(std::floor(_pos));

        symbol = buffer[idx];

        mid_idx = (idx >= _sps / 2) ? (idx - _sps / 2) : 0;

        mid = buffer[mid_idx];

        decision = (std::real(symbol) > 0) ? 1 : (std::real(symbol) < 0 ? -1 : 0);

        err = (_prev_decision - decision) * std::real(mid);

        _pos += _sps + _gain * err;

        _prev_decision = decision;

        out.push_back(symbol);
    }

    _pos -= total_len - keep;
}

void Core::Synchronizers::SymbolSync::gardner(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    if (len == 0) return;

    out.clear();

    size_t total_len = _history.size() + len;

    std::vector<std::complex<float>> buffer(total_len);

    std::copy(_history.begin(), _history.end(), buffer.begin());

    std::copy(in, in + len, buffer.begin() + _history.size());

    // Update the history with the new samples
    const size_t keep = std::min(total_len, static_cast<size_t>(_sps));

    _history.assign(buffer.end() - keep, buffer.end());

    std::complex<float> symbol, mid;
    float err = 0.0f, power = 0.0f;
    size_t idx, mid_idx;

    while (_pos < total_len) {
        // Round _pos to the nearest integer to get the current symbol index
        idx = static_cast<size_t>(std::floor(_pos));

        if (idx < static_cast<size_t>(_sps / 2)) {
            _pos += _sps;

            continue;
        }

        symbol = buffer[idx];

        mid_idx = (idx >= _sps / 2) ? (idx - _sps / 2) : 0;

        mid = buffer[mid_idx];

        err = std::real((_prev_symbol - symbol) * std::conj(mid));

        power = 0.5f * (std::norm(_prev_symbol) + std::norm(symbol)) + 1e-12f;

        err /= power;

        _pos += _sps + _gain * err;

        _prev_symbol = symbol;

        out.push_back(symbol);
    }

    _pos -= total_len - keep;
}

void Core::Synchronizers::SymbolSync::mueller_and_muller(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    //
}

void Core::LevelControllers::AGC::process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    if (len == 0) return;

    out.clear();

    out.resize(len);

    float temp, rate;

    for (size_t i = 0; i < len; ++i) {
        // Lower or raise the signal amplitude based on the current gain
        out[i] = in[i] * _gain;

        // Calculate the error between the target amplitude and the current output amplitude
        temp = -_target + std::abs(out[i]);

        rate = (temp > 0) ? _attack : _release;

        _gain -= rate * temp;

        if (_gain < 0) _gain = 10e-5;

        if (_gain > _maxGain) _gain = _maxGain;
    }
}

void Core::LevelControllers::AGC::reset() {
    _gain = 1.0f;
    _rms = 0.0f;
}

void Core::LevelControllers::AGC::set_target(float target) {
    _target = target;
}

void Core::LevelControllers::AGC::set_attack(float attack) {
    _attack = attack;
}

void Core::LevelControllers::AGC::set_release(float release) {
    _release = release;
}

void Core::LevelControllers::AGC::set_max_gain(float maxGain) {
    _maxGain = maxGain;
}

Core::Resamplers::RationalResampler::RationalResampler(uint32_t up, uint32_t down, uint32_t fs)
    : _up(up), _down(down), _fs(fs) {
    init();
}

void Core::Resamplers::RationalResampler::set_up(uint32_t up) {
    _up = up;

    _initialized = false;
}

void Core::Resamplers::RationalResampler::set_down(uint32_t down) {
    _down = down;

    _initialized = false;
}

void Core::Resamplers::RationalResampler::set_fs(uint32_t fs) {
    _fs = fs;

    _initialized = false;
}

void Core::Resamplers::RationalResampler::reset() {
    _k = 0;
    _H = 0;
    _start = 0;
    _history.clear();

    _initialized = false;
}

void Core::Resamplers::RationalResampler::init() {
    uint32_t gcd = std::gcd(_up, _down);

    _up /= gcd;
    _down /= gcd;

    float in_nyq = 0.5f * static_cast<float>(_fs);
    float out_nyq = 0.5f * static_cast<float>(_fs) * static_cast<float>(_up) / static_cast<float>(_down);
    float cutoff_hz = std::min(in_nyq, out_nyq);

    std::vector<float> taps = Signal::Utils::make_lowpass(_fs * _up, 0.8f * cutoff_hz, cutoff_hz);

    for (float& tap : taps) {
        tap *= static_cast<float>(_up);
    }

    _half_len = (taps.size() - 1) / 2;

    _H = (taps.size() -1) / _up;

    _taps = std::move(taps);

    _history.resize(_H);

    _k = _half_len;

    _initialized = true;
}

void Core::Resamplers::RationalResampler::process(const std::complex<float>* in, size_t len, std::vector<std::complex<float>>& out) {
    if (len == 0) return;

    if (!_initialized) {
        init();
    }

    out.clear();

    // Append history to the beginning of the input buffer
    std::vector<std::complex<float>> extended(_history.begin(), _history.end());

    extended.insert(extended.end(), in, in + len);

    size_t base, phase, j, n;
    std::complex<float> acc = 0;

    while (true) {
        // The first real sample that lands on tap
        base = _k / _up;

        // The first tap that lands on real sample (real = non zero)
        phase = _k % _up;

        // The base is at the future chunk of data
        if (base >= len) break;

        acc = 0;
        j = phase;
        n = base;

        while (j < _taps.size()) {
            // Convoluate
            acc += _taps[j] * extended[_H + n];

            // Advance to the next tap that is "up" steps away
            j += _up;

            // The next real sample is the one before the current one
            n -= 1;
        }

        // Downsample: move to the next output sample
        _k += _down;

        out.push_back(acc);
    }

    // Update _k so it starts at the correct position for the next input chunk
    _k -= len * _up;

    // Update history
    if (extended.size() >= _H) {
        std::copy(extended.end() - _H, extended.end(), _history.begin());
    }
}

void Core::Modulators::BPSK::process(const std::complex<float>* in, size_t len, std::vector<uint8_t>& out) {
    if (len == 0) return;

    out.clear();

    size_t total_len = _hasLastBit ? len + 1 : len;

    std::vector<uint8_t> bits(total_len);

    if (_hasLastBit) {
        bits[0] = _lastBit;
    }

    for (size_t i = 0; i < len; ++i) {
        bits[i + (_hasLastBit ? 1 : 0)] = (std::real(in[i]) > 0) ? 1 : 0;
    }

    out.resize(total_len - 1);

    for (size_t i = 1; i < total_len; ++i) {
        out[i - 1] = bits[i] ^ bits[i - 1];
    }

    _lastBit = bits[total_len - 1];
    _hasLastBit = true;
}

void Core::Modulators::BPSK::reset() {
    _lastBit = 0;
    _hasLastBit = false;
}

uint32_t Core::RDS::Decoder::crc(uint32_t word, size_t len) {
    // Divide the input word by POLY
    // The remainder after dividing by the polynomial is the CRC

    uint32_t reg = 0;

    // The lowest 10-bit of the remainder is the CRC
    size_t mlen = 10;

    for (int i = len - 1; i >= 0; --i) {
        // Shift in the next bit of the input word into the register
        reg = (reg << 1) | ((word >> i) & 0x01);

        // If the highest bit of the register is set, XOR with the polynomial to reduce it
        // 1 << mlen is the bit mask for the highest bit of the register (the bit beyond the CRC length)
        if (reg & (1 << mlen)) {
            reg ^= CRC_POLY;
        }
    }

    for (int i = 0; i < mlen; ++i) {
        // Push mlen zeros into reg
        reg = reg << 1;

        if (reg & (1 << mlen)) {
            reg ^= CRC_POLY;
        }
    }

    // Return the lowest mlen bits of the register as the CRC
    return reg & ((1 << mlen) - 1);
}

void Core::RDS::Decoder::process(const uint8_t* in, size_t len, std::vector<uint64_t>& out) {
    if (len == 0) return;

    out.clear();

    size_t idx = 0;

    while (idx < len) {
        // Shift in the next bit from the input data into the register
        _reg = (_reg << 1) | (in[idx] & 0x01);

        idx++;

        _bitCount++;

        if (!_synced) {
            uint32_t ccrc = crc(_reg, 26);

            // CRC returns an offset word
            // Check whether this offset word matches any
            for (int i = 0; i < 5; ++i) {
                if (ccrc == SYNDROME[i]) {
                    if (!_preSynced) {
                        _preSynced = true;
                        _lastSeenOffset = i;
                        _lastSeenOffsetCounter = _bitCount;
                    } else {
                        // Compute block distance from the currently seen block to the previous one
                        // Case when the currently seen block is at the edge, example: the last seen block is block D
                        // So we expect the next block is A
                        // But block A has OFFSET_POS lower than block D, so we need to account for the wrap-around.
                        if (OFFSET_POS[_lastSeenOffset] >= OFFSET_POS[i]) {
                            _blockDistance = (OFFSET_POS[i] + 4 - OFFSET_POS[_lastSeenOffset]) % 4;
                        } else {
                            _blockDistance = (OFFSET_POS[i] - OFFSET_POS[_lastSeenOffset]) % 4;
                        }

                        // Check if we actually have a valid block (26 bits apart)
                        if (_blockDistance * 26 != (_bitCount - _lastSeenOffsetCounter)) {
                            _preSynced = false;
                        } else {
                            _synced = true;

                            _blocksCounter = 0;
                            _wrongBlksCounter = 0;
                            _blockBitCounter = 0;
                            _blockNumber = (OFFSET_POS[i] + 1) % 4;
                            _groupAssemblyStarted = false;
                        }
                    }
                    
                    break;
                }
            }
        } else {
            // Wait until we have collected enough bits for a full block
            _blockBitCounter++;

            if (_blockBitCounter < 26) {
                continue;
            }

            bool goodBlock = false;
            uint32_t dataword = (_reg >> 10) & 0xFFFF; // Extract the 16-bit dataword from the 26-bit block
            uint32_t ccrc = crc(dataword, 16);
            uint32_t checkword = _reg & 0x3FF; // Extract the 10-bit checkword from the 26-bit block

            if (_blockNumber == 2) {
                // Edge case for C and C'
                uint32_t receivedCRC = checkword ^ OFFSET_WORDS[_blockNumber];

                if (receivedCRC == ccrc) {
                    // Good block
                    goodBlock = true;
                } else {
                    // Check if this is C'
                    uint32_t receivedCRC = checkword ^ OFFSET_WORDS[4];

                    if (receivedCRC == ccrc) {
                        // Good block (C')
                        goodBlock = true;
                    } else {
                        _wrongBlksCounter++;
                        goodBlock = false;
                    }
                }
            } else {
                uint32_t receivedCRC = checkword ^ OFFSET_WORDS[_blockNumber];

                if (receivedCRC == ccrc) {
                    // Good block
                    goodBlock = true;
                } else {
                    _wrongBlksCounter++;
                    goodBlock = false;
                }
            }

            // Done checking CRC
            if (_blockNumber == 0 && goodBlock) {
                _groupAssemblyStarted = true;
                _groupGoodBlocksCounter = 0;
                _group = 0;
            }

            if (_groupAssemblyStarted && goodBlock) {
                uint64_t shiftedDataword = static_cast<uint64_t>(dataword) << (48 - _blockNumber * 16);
                _group |= shiftedDataword;
                _groupGoodBlocksCounter++;

                if (_groupGoodBlocksCounter == 4) {
                    // Process the complete group
                    out.push_back(_group);
                    _groupAssemblyStarted = false;
                    _groupGoodBlocksCounter = 0;
                    _group = 0;
                }
            }

            _blockBitCounter = 0;
            _blockNumber = (_blockNumber + 1) % 4;
            _blocksCounter++;

            if (_blocksCounter >= 50) {
                if (_wrongBlksCounter >= 35) {
                    // Too many wrong blocks, reset counters
                    _synced = false;
                    _preSynced = false;
                }

                _blocksCounter = 0;
                _wrongBlksCounter = 0;
            }
        }
    }
}

void Core::RDS::Decoder::reset() {
    _reg = 0;
    _group = 0;
    _bitCount = 0;
    _lastSeenOffset = -1;
    _lastSeenOffsetCounter = -1;
    _wrongBlksCounter = 0;
    _blockBitCounter = 0;
    _blockNumber = 0;
    _blocksCounter = 0;
    _groupGoodBlocksCounter = 0;
    _blockDistance = 0;
    _synced = false;
    _preSynced = false;
    _groupAssemblyStarted = false;
}

void Core::RDS::Parser::reset() {
    _radioTextABFlag = 0;
    _rdsInfo = RDSInfo();
}

Core::RDS::Parser::Parser(RDSLocale locale) : _locale(locale) {
    reset();
}

void Core::RDS::Parser::process(const uint64_t* rds, size_t len, RDSInfo& info) {
    if (len == 0) return;

    uint16_t A, B, C, D;
    uint8_t groupType;
    bool ABFlag;
    int programId, programType, piAreaCoverage;
    
    for (size_t i = 0; i < len; ++i) {
        A = (rds[i] >> 48) & 0xFFFF;
        B = (rds[i] >> 32) & 0xFFFF;
        C = (rds[i] >> 16) & 0xFFFF;
        D = rds[i] & 0xFFFF;

        groupType = (B >> 12) & 0xF;

        ABFlag = (B >> 11) & 0x1;

        programId = A; // PI

        programType = (B >> 5) & 0x1F; // PTY

        strncpy(_rdsInfo.pty, PTY_TABLES[programType][_locale == RDSLocale::Europe ? 0 : 1], sizeof(_rdsInfo.pty) - 1);

        _rdsInfo.groupType = groupType;

        _rdsInfo.piProgramRefNumber = programId & 0xFF;

        piAreaCoverage = (programId >> 8) & 0xF;

        strncpy(_rdsInfo.coverageArea, COVERAGE_AREA_CODES[piAreaCoverage], sizeof(_rdsInfo.coverageArea) - 1);

        if (groupType != 2) continue;

        if (_radioTextABFlag != ((B >> 4) & 0x1)) {
            _radioTextABFlag = (B >> 4) & 0x1;

            // Flush text
            // Set radioText to empty string
            memset(_rdsInfo.radioText, 0, sizeof(_rdsInfo.radioText));
        }

        uint16_t textSegment = B & 0xF;

        if (ABFlag) {
            _rdsInfo.radioText[textSegment * 2] = (D >> 8) & 0xFF;
            _rdsInfo.radioText[textSegment * 2 + 1] = D & 0xFF;
        } else {
            _rdsInfo.radioText[textSegment * 4] = (C >> 8) & 0xFF;
            _rdsInfo.radioText[textSegment * 4 + 1] = C & 0xFF;
            _rdsInfo.radioText[textSegment * 4 + 2] = (D >> 8) & 0xFF;
            _rdsInfo.radioText[textSegment * 4 + 3] = D & 0xFF;
        }
    }

    // Copy rdsInfo to the output info
    info.locale = _locale;
    info.piProgramRefNumber = _rdsInfo.piProgramRefNumber;
    info.groupType = _rdsInfo.groupType;
    memcpy(info.pty, _rdsInfo.pty, sizeof(_rdsInfo.pty));
    memcpy(info.radioText, _rdsInfo.radioText, sizeof(_rdsInfo.radioText));
    memcpy(info.coverageArea, _rdsInfo.coverageArea, sizeof(_rdsInfo.coverageArea));
}