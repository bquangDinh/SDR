#ifndef SDR_SETTINGS_MANAGER_H
#define SDR_SETTINGS_MANAGER_H

#include "core/singleton.h"
#include "core/Constants.h"
#include "core/setting_subject.h"

class SdrSettingsManager : public Singleton<SdrSettingsManager>
{
    // enable to base class Singleton to call private/protected methods
    friend class Singleton<SdrSettingsManager>;

public:
    // Each setting has a getter, a setter, and an on_*_changed() accessor for its SettingSubject:
    //   auto id = SdrSettingsManager::instance().on_center_freq_changed().subscribe([](const uint32_t& v) { ... });
    //   SdrSettingsManager::instance().on_center_freq_changed().unsubscribe(id);
    // Subscribers run on the thread that called the setter, and only when the value changes.
#define SDR_SETTING(Type, getter, setter, subject_name, member) \
    Type getter() const { return member.get(); } \
    void setter(Type value) { member.set(value); } \
    SettingSubject<Type>& subject_name() { return member; }

    SDR_SETTING(int, source, set_source, on_source_changed, _source)
    SDR_SETTING(int, selectedDeviceIndex, set_selected_device_index, on_selected_device_index_changed, _selectedDeviceIndex)
    SDR_SETTING(int, rfGain, set_rf_gain, on_rf_gain_changed, _rfGain)
    SDR_SETTING(uint32_t, centerFreq, set_center_freq, on_center_freq_changed, _centerFreq)
    SDR_SETTING(double, offsetFreq, set_offset_freq, on_offset_freq_changed, _offsetFreq)
    SDR_SETTING(uint32_t, sampleRate, set_sample_rate, on_sample_rate_changed, _sampleRate)
    SDR_SETTING(int, correctionPPM, set_correction_ppm, on_correction_ppm_changed, _correctionPPM)
    SDR_SETTING(uint32_t, shiftAmount, set_shift_amount, on_shift_amount_changed, _shiftAmount)
    SDR_SETTING(int, samplingMode, set_sampling_mode, on_sampling_mode_changed, _samplingMode)
    SDR_SETTING(bool, rtlAgc, set_rtl_agc, on_rtl_agc_changed, _rtlAgc)
    SDR_SETTING(bool, offsetTuning, set_offset_tuning, on_offset_tuning_changed, _offsetTuning)
    SDR_SETTING(bool, tunerAgc, set_tuner_agc, on_tuner_agc_changed, _tunerAgc)
    SDR_SETTING(bool, biasTee, set_bias_tee, on_bias_tee_changed, _biasTee)
    SDR_SETTING(bool, shift, set_shift, on_shift_changed, _shift)
    SDR_SETTING(bool, correctIQ, set_correct_iq, on_correct_iq_changed, _correctIQ)
    SDR_SETTING(bool, invertSpectrum, set_invert_spectrum, on_invert_spectrum_changed, _invertSpectrum)
    SDR_SETTING(int, demodMode, set_demod_mode, on_demod_mode_changed, _demodMode)
    SDR_SETTING(uint32_t, bandwidth, set_bandwidth, on_bandwidth_changed, _bandwidth)
    SDR_SETTING(bool, squelch, set_squelch, on_squelch_changed, _squelch)
    SDR_SETTING(uint32_t, squelchAmount, set_squelch_amount, on_squelch_amount_changed, _squelchAmount)
    SDR_SETTING(uint32_t, cwShiftAmount, set_cw_shift_amount, on_cw_shift_amount_changed, _cwShiftAmount)
    SDR_SETTING(bool, binaural, set_binaural, on_binaural_changed, _binaural)
    SDR_SETTING(bool, antiFading, set_anti_fading, on_anti_fading_changed, _antiFading)
    SDR_SETTING(bool, lockCarrier, set_lock_carrier, on_lock_carrier_changed, _lockCarrier)
    SDR_SETTING(bool, stickyLock, set_sticky_lock, on_sticky_lock_changed, _stickyLock)
    SDR_SETTING(int, volume, set_volume, on_volume_changed, _volume)

#undef SDR_SETTING

private:
    SettingSubject<int> _source{-1};
    SettingSubject<int> _selectedDeviceIndex{-1};
    SettingSubject<int> _rfGain{0};
    SettingSubject<uint32_t> _centerFreq{162475000};
    SettingSubject<double> _offsetFreq{0};
    SettingSubject<uint32_t> _sampleRate{Constants::SampleRate::RATE_2_4_MSPS};
    SettingSubject<int> _correctionPPM{0};
    SettingSubject<uint32_t> _shiftAmount{0};
    SettingSubject<int> _samplingMode{Constants::SamplingMode::QUADRATURE_SAMPLING};
    SettingSubject<bool> _rtlAgc{false};
    SettingSubject<bool> _offsetTuning{false};
    SettingSubject<bool> _tunerAgc{false};
    SettingSubject<bool> _biasTee{false};
    SettingSubject<bool> _shift{false};
    SettingSubject<bool> _correctIQ{false};
    SettingSubject<bool> _invertSpectrum{false};
    SettingSubject<int> _demodMode{Constants::DemodulateMode::NFM};
    SettingSubject<uint32_t> _bandwidth{8000};
    SettingSubject<bool> _squelch{false};
    SettingSubject<uint32_t> _squelchAmount{0};
    SettingSubject<uint32_t> _cwShiftAmount{0};
    SettingSubject<bool> _binaural{false};
    SettingSubject<bool> _antiFading{false};
    SettingSubject<bool> _lockCarrier{false};
    SettingSubject<bool> _stickyLock{false};
    SettingSubject<int> _volume{50};

    SdrSettingsManager() = default; // only Singleton class can construct it

    ~SdrSettingsManager() = default;
};

#endif // SDR_SETTINGS_MANAGER_H
