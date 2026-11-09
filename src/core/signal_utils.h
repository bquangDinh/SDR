#ifndef SIGNAL_UTILS_H
#define SIGNAL_UTILS_H

#include <stdint.h>
#include <vector>
#include <complex>

enum class WindowType {
    Rectangular,
    Hamming,
    Hann,
    Blackman,
    Kaiser,
    Bartlett,
    BlackmanHarris,
};

struct filter_window_arg {
    WindowType windowType;
    double beta;
};

struct kaiser_ret_params {
    int numTaps;
    float beta;
};

namespace Signal {
    namespace General {
        // void shift_freq_to_zero_complex(
        //     const std::complex<float>* data,
        //     size_t size,
        //     float offsetHz,
        //     uint32_t sampleRate,
        //     std::vector<std::complex<float>>& out
        // );

        // template <typename T>
        // std::vector<T> decimate(const T* data, size_t size, size_t d);

        // template <typename T>
        // void upsample(const T* data, size_t size, size_t d, std::vector<T>& out);
    }

    namespace Filters {
        kaiser_ret_params kaisercord(float ripple, float width, uint32_t sampleRate);
        void firwin(
            std::vector<float>& out,
            int numTaps,
            double cutoff,
            filter_window_arg window,
            uint32_t sampleRate
        );
    }

    namespace Utils {
        std::vector<float> make_lowpass(uint32_t sampleRate, uint32_t passband, uint32_t stopband);

        /**
         * Resamples a complex signal by the given up and down factors.
         * 
         * @param data Pointer to the input complex signal.
         * @param size Number of elements in the input signal.
         * @param up Upsampling factor.
         * @param down Downsampling factor.
         * @param out Vector to store the resampled output signal.
         */
        void resample_cc_f(const std::complex<float>* data, size_t size, uint32_t up, uint32_t down, uint32_t fs, std::vector<std::complex<float>>& out);

        void convolute_cc_f(const std::complex<float>* data, size_t size, const float* taps, size_t numTaps, std::vector<std::complex<float>>& out);
    }

    namespace Internal {
        namespace Filters {
            float windowValue(filter_window_arg window, int n, int N);

            double besselI0(double x, int iters = 1000);

            double sinc(int n, int M, double cutoff);
        }
    }
}

#endif // SIGNAL_UTILS_H
