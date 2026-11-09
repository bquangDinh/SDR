#include "rtl_rds_thread.h"

#include "core/sdr_settings_manager.h"
#include "core/Constants.h"
#include "core/signal_utils.h"
#include <iostream>
#include <numeric>

namespace {
constexpr size_t RDS_SPECTRUM_FFT_SIZE = 1024;
}

Resampler::Resampler(uint32_t up, uint32_t down, uint32_t fs)
    : _upRate(up), _downRate(down), _fs(fs) {
    uint32_t gcd = std::gcd(_upRate, _downRate);

    _upRate /= gcd;
    _downRate /= gcd;

    float in_nyq = 0.5f * static_cast<float>(fs);
    float out_nyq = 0.5f * static_cast<float>(_fs) * static_cast<float>(_upRate) / static_cast<float>(_downRate);
    float cutoff_hz = std::min(in_nyq, out_nyq);
    
    // stopband at cutoff frequency
    std::vector<float> taps = Signal::Utils::make_lowpass(fs * _upRate, 0.8f * cutoff_hz, cutoff_hz);

    for (float& tap : taps) {
        tap *= static_cast<float>(_upRate);
    }

    _half_len = (taps.size() - 1) / 2;

    _H = (taps.size() - 1) / _upRate;

    _taps = std::move(taps);

    _hist.resize(_H);

    _k = _half_len;
}

void Resampler::process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    out.clear();

    std::vector<std::complex<float>> upsampled(len * _upRate, std::complex<float>(0.0f, 0.0f));

    for (size_t i = 0; i < len; ++i) {
        upsampled[i * _upRate] = data[i];
    }

    std::vector<std::complex<float>> filtered;

    Signal::Utils::convolute_cc_f(upsampled.data(), upsampled.size(), _taps.data(), _taps.size(), filtered);

    // Downsample the filtered signal by taking every _downRate-th sample
    size_t i = _bufPos;

    for (; i < filtered.size(); i += _downRate) {
        out.push_back(filtered[i]);
    }

    _bufPos = i - filtered.size();
}

void Resampler::process_optimal(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    out.clear();

    // Append the history into the beginning of the input data
    std::vector<std::complex<float>> extended_data(_hist.begin(), _hist.end());

    extended_data.insert(extended_data.end(), data, data + len);

    while (true) {
        size_t base = _k / _upRate;
        size_t phase = _k % _upRate;

        if (base >= len) {
            break;
        }

        std::complex<float> acc = 0;
        size_t j = phase;
        int64_t n = base;

        while (j < _taps.size()) {
            acc += _taps[j] * extended_data[_H + n];
            j += _upRate;
            n -= 1;
        }

        _k += _downRate;

        out.push_back(acc);
    }

    // Update _k so it starts at the correct position for the next chunk of data
    _k -= len * _upRate;

    // Update the history buffer with the last _half_len samples of the extended data
    if (extended_data.size() >= _H) {
        std::copy(extended_data.end() - _H, extended_data.end(), _hist.begin());
    }
}

void CFC::process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    if (_buf.size() < CFC::MIN_BUF_SIZE) {
        _buf.insert(_buf.end(), data, data + len);

        _mixer.set_offset_freq(-_lastFreq);

        _mixer.set_sample_rate(_fs);

        _mixer.process(_buf.data(), _buf.size(), out);
        
        return;
    }
    
    std::vector<std::complex<float>> buf(CFC::MIN_BUF_SIZE);

    std::vector<std::complex<float>> squared(CFC::MIN_BUF_SIZE);

    std::copy(_buf.begin(), _buf.begin() + CFC::MIN_BUF_SIZE, buf.begin());

    _buf.erase(_buf.begin(), _buf.begin() + CFC::MIN_BUF_SIZE);

    // Square the samples to remove data modulation and emphasize frequency components
    for (size_t i = 0; i < buf.size(); ++i) {
        squared[i] = buf[i] * buf[i];
    }

    std::vector<float> fft_out;

    // Spectrum::compute needs at least one full block of FFT bins,
    // otherwise it leaves the output empty; reuse the last estimate then.
    _spectrum.compute(squared.data(), squared.size(), fft_out);

    if (!fft_out.empty()) {
        // Find what frequency corresponds to each FFT bin
        float bin_freq = _fs / static_cast<float>(CFC::FFT_SIZE);

        size_t maxFreqIndex = 0;

        for (size_t i = 1; i < CFC::FFT_SIZE; ++i) {
            if (fft_out[i] > fft_out[maxFreqIndex]) {
                maxFreqIndex = i;
            }
        }

        // Output is fft-shifted, so DC is at bins / 2
        _lastFreq = 0.5f * (static_cast<int>(maxFreqIndex) - static_cast<int>(CFC::FFT_SIZE / 2)) * bin_freq;

        std::cout << "Detected peak frequency: " << _lastFreq << std::endl;
    } else {
        std::cout << "FFT output empty, using last frequency: " << _lastFreq << std::endl;
    }

    // The peak is the frequency offset we want to remove
    _mixer.set_offset_freq(-_lastFreq);

    _mixer.set_sample_rate(_fs);

    _mixer.process(buf.data(), buf.size(), out);
}

void AGC::process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    out.resize(len);

    for (size_t i = 0; i < len; ++i) {
        out[i] = data[i] * _gain;

        float temp = -_target + std::abs(out[i]);

        float rate = (temp > 0) ? _attack : _release;

        _gain -= temp * rate;

        if (_gain < 0) _gain = 10e-5;

        if (_gain > _maxGain) _gain = _maxGain;
    }
}

void BPSKDemod::process(const std::complex<float>* data, size_t len, std::vector<uint8_t>& out) {
    if (len == 0) return;

    const bool hadLastBit = _hasLastBit;
    size_t total_len = hadLastBit ? len + 1 : len;

    std::vector<uint8_t> bits(total_len);

    if (hadLastBit) {
        bits[0] = _lastBit;
    }

    for (size_t i = 0; i < len; ++i) {
        bits[i + (hadLastBit ? 1 : 0)] = (std::real(data[i]) > 0) ? 1 : 0;
    }

    out.resize(total_len - 1);

    for (size_t i = 1; i < total_len; ++i) {
        out[i - 1] = bits[i] ^ bits[i - 1];
    }
    
    _lastBit = bits[total_len - 1];
    _hasLastBit = true;
}

SymbolSync::SymbolSync(int sps, float gain) {
    _sps = sps;
    _gain = gain;
    _pos = sps;
    _prev_decision = 0;
    _prev_samples.clear();
}

void SymbolSync::process(const std::complex<float>* data, size_t len, SymbolSyncMethod method, std::vector<std::complex<float>>& out) {
    switch (method) {
        case SymbolSyncMethod::ZeroCrossing:
            zero_crossing(data, len, out);
            break;
        case SymbolSyncMethod::Mueller:
            mueller(data, len, out);
            break;
        case SymbolSyncMethod::Gardner:
            gardner(data, len, out);
            break;
    }
}

void SymbolSync::zero_crossing(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    size_t total_len = _prev_samples.size() + len;

    std::vector<std::complex<float>> buffer(total_len);

    // Copy previous samples to the buffer
    std::copy(_prev_samples.begin(), _prev_samples.end(), buffer.begin());

    // Copy new data to the buffer
    std::copy(data, data + len, buffer.begin() + _prev_samples.size());

    // Update prev_samples for the next call
    const size_t keep = std::min<size_t>(_sps, total_len);

    _prev_samples.assign(buffer.end() - keep, buffer.end());

    std::complex<float> symbol, mid;
    int decision;
    float err;

    while (_pos < total_len) {
        // Round _pos to the nearest integer to get the sample index
        size_t idx = static_cast<size_t>(std::floor(_pos));

        // Pick the sample at the calculated index that we think best represents the symbol
        symbol = buffer[idx];
        
        // Pick the midpoint between the current sample and the previous one
        size_t mid_idx = (idx >= _sps / 2) ? idx - _sps / 2 : 0;

        mid = buffer[mid_idx];

        decision = (std::real(symbol) > 0) ? 1 : (std::real(symbol) < 0 ? -1 : 0);

        err = (_prev_decision - decision) * std::real(mid);

        _pos += _sps + _gain * err;

        _prev_decision = decision;

        out.push_back(symbol);
    }

    // The next buffer starts with the kept samples, so rebase _pos against them
    _pos -= total_len - keep;
}

void SymbolSync::mueller(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    // Implement Mueller symbol synchronization here
}

void SymbolSync::gardner(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    size_t total_len = _prev_samples.size() + len;

    std::vector<std::complex<float>> buffer(total_len);

    std::copy(_prev_samples.begin(), _prev_samples.end(), buffer.begin());
    std::copy(data, data + len, buffer.begin() + _prev_samples.size());

    const size_t keep = std::min<size_t>(_sps, total_len);

    _prev_samples.assign(buffer.end() - keep, buffer.end());

    while (_pos < total_len) {
        size_t idx = static_cast<size_t>(std::floor(_pos));

        // Not enough history for the midpoint (only happens at the very start)
        if (idx < static_cast<size_t>(_sps / 2)) {
            _pos += _sps;
            continue;
        }

        std::complex<float> symbol = buffer[idx];
        std::complex<float> mid    = buffer[idx - _sps / 2];

        // Gardner timing error: no decisions, works at any carrier phase
        float err = std::real((_prev_symbol - symbol) * std::conj(mid));

        // Normalize by signal power so _gain doesn't depend on the signal level
        float power = 0.5f * (std::norm(_prev_symbol) + std::norm(symbol)) + 1e-12f;

        err /= power;

        _pos += _sps + _gain * err;

        _prev_symbol = symbol;

        out.push_back(symbol);
    }

    _pos -= total_len - keep;
}

CostaLoop::CostaLoop(float alpha, float beta) {
    _alpha = alpha;
    _beta = beta;
    _phase = 0;
    _freq = 0;
}

void CostaLoop::process(const std::complex<float>* data, size_t len, std::vector<std::complex<float>>& out) {
    out.resize(len);

    float err;

    for (size_t i = 0; i < len; ++i) {
        out[i] = data[i] * std::exp(std::complex<float>(0, -_phase));

        err = std::real(out[i]) * std::imag(out[i]);

        _freq += _beta * err;

        _phase += _freq + _alpha * err;

        // Keep _phase within the range [0, 2*pi) to avoid numerical issues
        if (_phase >= 2 * M_PI) _phase -= 2 * M_PI;
        else if (_phase < 0) _phase += 2 * M_PI;
    }
}

uint32_t RDSDecoder::crc(uint32_t word, size_t len) {
    const uint32_t poly = 0x5B9;
    uint32_t reg = 0;
    size_t mlen = 10;

    for (int i = len - 1; i >= 0; --i) {
        reg = (reg << 1) | ((word >> i) & 0x01);

        if (reg & (1 << mlen)) {
            reg ^= poly;
        }
    }

    for (int i = 0; i < mlen; ++i) {
        reg = reg << 1;

        if (reg & (1 << mlen)) {
            reg ^= poly;
        }
    }

    return reg & ((1 << mlen) - 1);
}

void RDSDecoder::process(const uint8_t* data, size_t len, std::vector<uint64_t>& out) {
    out.clear();

    size_t idx = 0;

    while (idx < len) {
        // Shift in the next bit from the input data into the register
        reg = (reg << 1) | (data[idx] & 0x01);

        idx++;

        bitCount++;

        if (!synced) {
            uint32_t ccrc = crc(reg, 26);

            // CRC returns an offset word
            // Check whether this offset word matches any
            for (int i = 0; i < 5; ++i) {
                if (ccrc == SYNDROME[i]) {
                    if (!preSynced) {
                        preSynced = true;
                        lastseenOffset = i;
                        lastseenOffsetCounter = bitCount;
                    } else {
                        // Compute block distance from the currently seen block to the previous one
                        // Case when the currently seen block is at the edge, example: the last seen block is block D
                        // So we expect the next block is A
                        // But block A has OFFSET_POS lower than block D, so we need to account for the wrap-around.
                        if (OFFSET_POS[lastseenOffset] >= OFFSET_POS[i]) {
                            blockDistance = (OFFSET_POS[i] + 4 - OFFSET_POS[lastseenOffset]) % 4;
                        } else {
                            blockDistance = (OFFSET_POS[i] - OFFSET_POS[lastseenOffset]) % 4;
                        }

                        // Check if we actually have a valid block (26 bits apart)
                        if (blockDistance * 26 != (bitCount - lastseenOffsetCounter)) {
                            preSynced = false;
                        } else {
                            synced = true;

                            blocksCounter = 0;
                            wrongBlksCounter = 0;
                            blockBitCounter = 0;
                            blockNumber = (OFFSET_POS[i] + 1) % 4;
                            groupAssemblyStarted = false;
                        }
                    }
                    
                    break;
                }
            }
        } else {
            // Wait until we have collected enough bits for a full block
            blockBitCounter++;

            if (blockBitCounter < 26) {
                continue;
            }

            bool goodBlock = false;
            uint32_t dataword = (reg >> 10) & 0xFFFF; // Extract the 16-bit dataword from the 26-bit block
            uint32_t ccrc = crc(dataword, 16);
            uint32_t checkword = reg & 0x3FF; // Extract the 10-bit checkword from the 26-bit block

            if (blockNumber == 2) {
                // Edge case for C and C'
                uint32_t receivedCRC = checkword ^ OFFSET_WORDS[blockNumber];

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
                        wrongBlksCounter++;
                        goodBlock = false;
                    }
                }
            } else {
                uint32_t receivedCRC = checkword ^ OFFSET_WORDS[blockNumber];

                if (receivedCRC == ccrc) {
                    // Good block
                    goodBlock = true;
                } else {
                    wrongBlksCounter++;
                    goodBlock = false;
                }
            }

            // Done checking CRC
            if (blockNumber == 0 && goodBlock) {
                groupAssemblyStarted = true;
                groupGoodBlocksCounter = 0;
                group = 0;
            }

            if (groupAssemblyStarted && goodBlock) {
                uint64_t shiftedDataword = static_cast<uint64_t>(dataword) << (48 - blockNumber * 16);
                group |= shiftedDataword;
                groupGoodBlocksCounter++;

                if (groupGoodBlocksCounter == 4) {
                    // Process the complete group
                    out.push_back(group);
                    groupAssemblyStarted = false;
                    groupGoodBlocksCounter = 0;
                    group = 0;
                }
            }

            blockBitCounter = 0;
            blockNumber = (blockNumber + 1) % 4;
            blocksCounter++;

            if (blocksCounter >= 50) {
                if (wrongBlksCounter >= 35) {
                    // Too many wrong blocks, reset counters
                    synced = false;
                    preSynced = false;
                }

                blocksCounter = 0;
                wrongBlksCounter = 0;
            }
        }
    }
}

void RDSParser::process(const uint64_t* rds_data, size_t len, std::string& outRadioText) {
    for (size_t i = 0; i < len; ++i) {
        uint64_t rds = rds_data[i];
        uint16_t A = (rds >> 48) & 0xFFFF;
        uint16_t B = (rds >> 32) & 0xFFFF;
        uint16_t C = (rds >> 16) & 0xFFFF;
        uint16_t D = rds & 0xFFFF;

        groupType = (B >> 12) & 0xF;

        bool ABFlag = (B >> 11) & 0x1;

        if (groupType != 2) continue;

        if (radioTextABFlag != ((B >> 4) & 0x1)) {
            radioTextABFlag = (B >> 4) & 0x1;

            // Flush text
            // Set radioText to empty string
            memset(radioText, 0, sizeof(radioText));
        }

        uint16_t textSegment = B & 0xF;

        if (ABFlag) {
            radioText[textSegment * 2] = (D >> 8) & 0xFF;
            radioText[textSegment * 2 + 1] = D & 0xFF;
        } else {
            radioText[textSegment * 4] = (C >> 8) & 0xFF;
            radioText[textSegment * 4 + 1] = C & 0xFF;
            radioText[textSegment * 4 + 2] = (D >> 8) & 0xFF;
            radioText[textSegment * 4 + 3] = D & 0xFF;
        }
    }

    // Copy the radio text to the output string
    outRadioText = std::string(radioText);
}

RtlRDSThread::RtlRDSThread(ChunkQueue* q, QObject* parent) :
    QThread(parent),
    _q(q)
{
    _sampleRateSubId = SdrSettingsManager::instance().on_sample_rate_changed().subscribe([this](const uint32_t& v) { this->on_sample_rate_changed(v); });
    _offsetFreqSubId = SdrSettingsManager::instance().on_offset_freq_changed().subscribe([this](const int64_t& v) { this->on_offset_freq_changed(v); });
}

void RtlRDSThread::pause() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _paused = true;
    }
}

void RtlRDSThread::resume() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _paused = false;
    }

    _cv.notify_all();
}

void RtlRDSThread::stop() {
    {
        std::lock_guard<std::mutex> lk(_m);

        _stop = true;
    }

    _cv.notify_all();
}

void RtlRDSThread::run() {
    _rdsSpectrum = std::make_unique<Spectrum>(RDS_SPECTRUM_FFT_SIZE);

    while (true) {
        {
            std::unique_lock<std::mutex> lk(_m);

            _cv.wait(lk, [&] {
                return !_paused || _stop;
            });

            if (_stop) break;
        }

        if (_dirty.exchange(false)) {
            std::cout << "Init" << std::endl;

            init();
        }

        read();
    }
}

void RtlRDSThread::init() {
    std::cout << "Initializing RtlRDSThread Params" << std::endl;

    _rdsSpectrumSamples.clear();

    uint32_t fs = SdrSettingsManager::instance().sampleRate();
    uint32_t offsetFreq = SdrSettingsManager::instance().offsetFreq();
    uint32_t passband = 100e3;
    uint32_t stopband = 110e3;
    uint32_t d1 = 10, d2 = 10, d3 = 1;

    _generalMixer = std::make_unique<Mixer>();

    _generalMixer->set_offset_freq(offsetFreq);

    _generalMixer->set_sample_rate(fs);

    std::cout << "General Mixer with offset frequency " << offsetFreq << " Hz and sample rate " << fs << " Hz" << std::endl;

    std::vector<float> taps1 = Signal::Utils::make_lowpass(fs, passband, stopband);

    _generalLowPass = std::make_unique<FastDecimator>(taps1, d1);

    std::cout << "General Low Pass with fs = " << fs << " Hz | passband = " << passband << " Hz | stopband = " << stopband << " Hz | Decimation = " << d1 << std::endl;

    fs = fs / d1;

    _cfc = std::make_unique<CFC>(fs);

    std::cout << "CFC with fs = " << fs << " Hz" << std::endl;

    _fm = std::make_unique<FmDiscriminator>(fs, 75000);

    std::cout << "FM Demod with fs = " << fs << " Hz | deviation = 75000 Hz" << std::endl;

    _mixer = std::make_unique<Mixer>();

    // RDS band at 57 kHz
    _mixer->set_offset_freq(57e3);

    _mixer->set_sample_rate(fs);

    std::cout << "RDS Mixer with offset frequency 57 kHz and sample rate " << fs << " Hz" << std::endl;

    passband = 2.5e3;
    stopband = 3e3;

    std::vector<float> taps2 = Signal::Utils::make_lowpass(fs, passband, stopband);

    _rdsLowPass = std::make_unique<FastDecimator>(taps2, d2);

    std::cout << "RDS Low Pass with fs = " << fs << " Hz | passband = " << passband << " Hz | stopband = " << stopband << " Hz | Decimation = " << d2 << std::endl;

    fs = fs / d2;

    // Assuming 16 samples / symbol at 19 kHz sample rate
    std::vector<float> rrc_taps(16);

    for (int i = 0; i < 8; ++i) {
        rrc_taps[i] = 1;
        rrc_taps[i + 8] = -1;
    }

    _manchesterLowPass = std::make_unique<FastDecimator>(rrc_taps, d3);

    std::cout << "Manchester Low Pass with fs = " << fs << " Hz | Decimation = " << d3 << std::endl;

    // _cfc and _fm run on the stream after the first decimation
    _agc = std::make_unique<AGC>(1.0f, 0.1f, 0.01f, 100.0f);
    _symbolSync = std::make_unique<SymbolSync>(16, 0.5f);
    _costaLoop = std::make_unique<CostaLoop>(0.3f, 0.02f);
    _bpsk = std::make_unique<BPSKDemod>();
    _rdsDecoder = std::make_unique<RDSDecoder>();
    _rdsParser = std::make_unique<RDSParser>();
    _resampler = std::make_unique<Resampler>(19e3, fs, fs);

    std::cout << "Resampler with input fs = " << 19e3 << " Hz | output fs = " << fs << " Hz" << std::endl;
}

void RtlRDSThread::read() {
    // If the demodulation is not WFM, why bother?
    if (SdrSettingsManager::instance().demodMode() != Constants::DemodulateMode::WFM) {
        // Drain the queue (blocking pop) instead of spinning or sleeping
        _q->pop(std::chrono::milliseconds(100));

        return;
    }

    if (Chunk c = _q->pop(std::chrono::milliseconds(100))) {
        _o1.clear();

        _o2.clear();

        _o3.clear();

        _generalMixer->process(c->data(), c->size(), _o1);

        // Filter
        _generalLowPass->process(_o1.data(), _o1.size(), _o2);

        _o1.clear();

        // Apply CFC to remove frequency offset
        // _cfc->process(_o2.data(), _o2.size(), _o1);

        // _o2.clear();

        // FM Demod
        _fm->process(_o2.data(), _o2.size(), _o3);

        _o1.clear();

        _o2.clear();

        for (size_t i = 0; i < _o3.size(); ++i) {
            _o2.push_back(std::complex<float>(_o3[i], 0.0f));
        }

        // // Emit RDS Spectrum every 30 ms
        // _rdsSpectrumSamples.insert(_rdsSpectrumSamples.end(), _o3.begin(), _o3.end());

        // // Emit RDS spectrum every 30 ms
        // const auto now = std::chrono::steady_clock::now();
        // if (now - _lastEmitRDSSpectrum >= std::chrono::milliseconds(30)
        //     && _rdsSpectrumSamples.size() >= RDS_SPECTRUM_FFT_SIZE) {
        //     std::vector<float> spectrum;

        //     _rdsSpectrum->compute(_rdsSpectrumSamples.data(), _rdsSpectrumSamples.size(), spectrum);

        //     if (!spectrum.empty()) {
        //         std::cout << "RDS spectrum computed with " << spectrum.size()
        //                   << " points | accumulated samples: " << _rdsSpectrumSamples.size() << std::endl;

        //         emit rdsSpectrumUpdated(QList<double>(spectrum.begin(), spectrum.end()));

        //         const size_t consumed = _rdsSpectrumSamples.size()
        //             - (_rdsSpectrumSamples.size() % RDS_SPECTRUM_FFT_SIZE);
        //         _rdsSpectrumSamples.erase(
        //             _rdsSpectrumSamples.begin(),
        //             _rdsSpectrumSamples.begin() + consumed);
        //         _lastEmitRDSSpectrum = now;
        //     }
        // }

        // Shift the RDS band to baseband 57 kHz
        _mixer->process(_o2.data(), _o2.size(), _o1);

        _o2.clear();

        // Filter RDS band
        _rdsLowPass->process(_o1.data(), _o1.size(), _o2);

        _o1.clear();

        // Resample at 19 kHz
        // fs / 100 is the sample rate after the RDS lowpass decimation
        // Ex: The original sample rate is 2.5 MHz, the first low-pass decimated it down that gives 250 kHz
        // The second rds low pass decimated further down to 25 kHz
        // "So the resample_cc_f call is converting from 25 kHz to 19 kHz
        // Signal::Utils::resample_cc_f(_o2.data(), _o2.size(), 19e3, fs / 100, fs / 100, _o1);
        _resampler->process_optimal(_o2.data(), _o2.size(), _o1);

        _o2.clear();

        // Apply Manchester
        _manchesterLowPass->process(_o1.data(), _o1.size(), _o2);

        _o1.clear();

        // Apply AGC
        _agc->process(_o2.data(), _o2.size(), _o1);

        _o2.clear();

        // Accumulate the processed samples
        _buf.insert(_buf.end(), _o1.begin(), _o1.end());

        _o1.clear();

        if (_buf.size() < 10000) {
            return;
        }

        // Copy the accumulated buffer to _o1 for further processing
        _o1.insert(_o1.end(), _buf.begin(), _buf.end());

        _buf.clear();

        _rdsSpectrumSamples.insert(_rdsSpectrumSamples.end(), _o1.begin(), _o1.end());

        // Emit RDS spectrum every 30 ms
        auto now = std::chrono::steady_clock::now();

        if (now - _lastEmitRDSSpectrum >= std::chrono::milliseconds(30)
            && _rdsSpectrumSamples.size() >= RDS_SPECTRUM_FFT_SIZE) {
            std::vector<float> spectrum;

            _rdsSpectrum->compute(_rdsSpectrumSamples.data(), _rdsSpectrumSamples.size(), spectrum);

            if (!spectrum.empty()) {
                emit rdsSpectrumUpdated(QList<double>(spectrum.begin(), spectrum.end()));

                const size_t consumed = _rdsSpectrumSamples.size()
                    - (_rdsSpectrumSamples.size() % RDS_SPECTRUM_FFT_SIZE);
                _rdsSpectrumSamples.erase(
                    _rdsSpectrumSamples.begin(),
                    _rdsSpectrumSamples.begin() + consumed);
                _lastEmitRDSSpectrum = now;
            }
        }

        // Symbol synchronization
        _symbolSync->process(_o1.data(), _o1.size(), SymbolSyncMethod::ZeroCrossing, _o2);

        _o1.clear();

        _costaLoop->process(_o2.data(), _o2.size(), _o1);

        _o2.clear();

        // Emit IQ stream every 30 ms
        if (std::chrono::steady_clock::now() - _lastEmit >= std::chrono::milliseconds(30)) {
            _iq.clear();

            _iq.insert(_iq.end(), _o1.begin(), _o1.end());
            
            emit IQUpdated(_iq);

            _lastEmit = std::chrono::steady_clock::now();
        }

        // BPSK Demod
        _bpskOut.clear();

        _bpsk->process(_o1.data(), _o1.size(), _bpskOut);

        // RDS Decoding
        _rdsData.clear();

        _rdsDecoder->process(_bpskOut.data(), _bpskOut.size(), _rdsData);

        // Parse RDS data
        _rdsParser->process(_rdsData.data(), _rdsData.size(), _radioText);

        std::cout << "Text is " << _radioText << std::endl;

        // Throttle: emit at most every 30 ms, only when text is non-empty and changed
        now = std::chrono::steady_clock::now();

        if (now - _lastEmitRadioText >= std::chrono::milliseconds(30)) {
            _lastEmitRadioText = now;

            if (!_radioText.empty() && _radioText != _lastEmittedText) {
                _lastEmittedText = _radioText;

                emit radioTextUpdated(_radioText);
            }
        }
    }
}

void RtlRDSThread::on_sample_rate_changed(const uint32_t& sampleRate) {
    _dirty = true;
}

void RtlRDSThread::on_offset_freq_changed(const int64_t& offsetFreq) {
    _dirty = true;
}