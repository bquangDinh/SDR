#include "rtl_rds_thread.h"

#include "core/sdr_settings_manager.h"
#include "core/Constants.h"
#include "core/signal_utils.h"
#include <iostream>

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

    // _cfc = std::make_unique<CFC>(fs);

    // std::cout << "CFC with fs = " << fs << " Hz" << std::endl;

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
    _agc = std::make_unique<Core::LevelControllers::AGC>(1.0f, 0.1f, 0.01f, 100.0f);
    _symbolSync = std::make_unique<Core::Synchronizers::SymbolSync>(16, 0.5f, Core::Synchronizers::SymbolSyncMethod::ZeroCrossing);
    _costaLoop = std::make_unique<Core::Synchronizers::CostasLoop>(0.3f, 0.02f);
    _bpsk = std::make_unique<Core::Modulators::BPSK>();
    _rdsDecoder = std::make_unique<Core::RDS::Decoder>();
    _rdsParser = std::make_unique<Core::RDS::Parser>(Core::RDS::RDSLocale::USA);
    _resampler = std::make_unique<Core::Resamplers::RationalResampler>(19e3, fs, fs);

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
        _generalMixer->process(c->data(), c->size(), _o1);

        // Filter
        _generalLowPass->process(_o1.data(), _o1.size(), _o2);

        // Apply CFC to remove frequency offset
        // _cfc->process(_o2.data(), _o2.size(), _o1);

        // FM Demod
        _fm->process(_o2.data(), _o2.size(), _o3);

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

        // Filter RDS band
        _rdsLowPass->process(_o1.data(), _o1.size(), _o2);

        // Resample at 19 kHz
        // fs / 100 is the sample rate after the RDS lowpass decimation
        // Ex: The original sample rate is 2.5 MHz, the first low-pass decimated it down that gives 250 kHz
        // The second rds low pass decimated further down to 25 kHz
        // "So the resample_cc_f call is converting from 25 kHz to 19 kHz
        // Signal::Utils::resample_cc_f(_o2.data(), _o2.size(), 19e3, fs / 100, fs / 100, _o1);
        _resampler->process(_o2.data(), _o2.size(), _o1);

        // Apply Manchester
        _manchesterLowPass->process(_o1.data(), _o1.size(), _o2);

        // Apply AGC
        _agc->process(_o2.data(), _o2.size(), _o1);

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
        _symbolSync->process(_o1.data(), _o1.size(), _o2);

        _costaLoop->process(_o2.data(), _o2.size(), _o1);

        // Emit IQ stream every 30 ms
        if (std::chrono::steady_clock::now() - _lastEmit >= std::chrono::milliseconds(30)) {
            _iq.clear();

            _iq.insert(_iq.end(), _o1.begin(), _o1.end());
            
            emit IQUpdated(_iq);

            _lastEmit = std::chrono::steady_clock::now();
        }

        // BPSK Demod
        _bpsk->process(_o1.data(), _o1.size(), _bpskOut);

        // RDS Decoding
        _rdsDecoder->process(_bpskOut.data(), _bpskOut.size(), _rdsData);

        // Parse RDS data
        _rdsParser->process(_rdsData.data(), _rdsData.size(), _rdsInfo);

        std::cout << "Text is " << _rdsInfo.radioText << std::endl;

        // Throttle: emit at most every 30 ms, only when text is non-empty and changed
        now = std::chrono::steady_clock::now();

        if (now - _lastEmitRDSInfo >= std::chrono::milliseconds(30)) {
            _lastEmitRDSInfo = now;

            emit rdsInfoUpdated(_rdsInfo);
        }
    }
}

void RtlRDSThread::on_sample_rate_changed(const uint32_t& sampleRate) {
    _dirty = true;
}

void RtlRDSThread::on_offset_freq_changed(const int64_t& offsetFreq) {
    _dirty = true;
}