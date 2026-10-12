#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdint.h>
#include <vector>
// #include <complex>

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
    }

    namespace Internal {
        namespace Filters {
            float windowValue(filter_window_arg window, int n, int N);

            double besselI0(double x, int iters = 1000);

            double sinc(int n, int M, double cutoff);
        }
    }
}

#endif // SIGNAL_H
