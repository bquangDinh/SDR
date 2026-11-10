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
#include "core/core.h"

class RtlRDSThread : public QThread
{
    Q_OBJECT
public:
    static constexpr size_t RDS_SPECTRUM_FFT_SIZE = 1024;

    RtlRDSThread(ChunkQueue* q, QObject* parent);

    void pause();

    void stop();

    void resume();

protected:
    void run() override;

signals:
    void rdsInfoUpdated(const Core::RDS::RDSInfo& rdsInfo);

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
    std::unique_ptr<Mixer> _mixer, _generalMixer;
    std::unique_ptr<Core::Modulators::BPSK> _bpsk;
    std::unique_ptr<Core::Synchronizers::SymbolSync> _symbolSync;
    std::unique_ptr<Core::RDS::Decoder> _rdsDecoder;
    std::unique_ptr<Core::RDS::Parser> _rdsParser;
    std::unique_ptr<Core::Synchronizers::CostasLoop> _costaLoop;
    std::unique_ptr<Core::LevelControllers::AGC> _agc;
    std::unique_ptr<Core::Resamplers::RationalResampler> _resampler;
    std::unique_ptr<Spectrum> _rdsSpectrum;

    ChunkQueue* _q = nullptr;
    std::mutex _m;
    std::condition_variable _cv;

    std::vector<std::complex<float>> _o1, _o2, _iq;
    std::vector<std::complex<float>> _rdsSpectrumSamples;
    std::vector<float> _o3;
    std::vector<std::complex<float>> _buf;
    std::vector<uint64_t> _rdsData;
    std::vector<uint8_t> _bpskOut;

    Core::RDS::RDSInfo _rdsInfo;
    std::chrono::steady_clock::time_point _lastEmit = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point _lastEmitRDSSpectrum = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point _lastEmitRDSInfo = std::chrono::steady_clock::now();

    void init();

    void read();

    void on_sample_rate_changed(const uint32_t& sampleRate);
    void on_offset_freq_changed(const int64_t& offsetFreq);
};

#endif // RTL_RDS_H
