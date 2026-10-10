#include "core/Signal.h"

#include <math.h>
#include "cassert"

kaiser_ret_params Signal::Filters::kaisercord(float ripple, float width, uint32_t sampleRate) {
    assert(ripple >= 8.0f && "attenuation must be at least 8 dB");

    float delta_angular_freq = 2 * static_cast<float>(M_PI) * width / sampleRate;

    int numTaps = static_cast<int>(std::ceil(((ripple - 7.95f) / (2.285f * delta_angular_freq)) + 1.0f));

    float beta = 0;

    if (ripple > 50) beta = 0.1102f * (ripple - 8.7f);
    else if (ripple >= 21) beta = 0.5842f * std::pow(ripple - 21, 0.4f) + 0.07886f * (ripple - 21);
    else beta = 0;

    return {numTaps, beta};
}

void Signal::Filters::firwin(
    std::vector<float>& out,
    int numTaps,
    double cutoff,
    filter_window_arg window,
    uint32_t sampleRate
    ) {
    assert(numTaps > 0 && "numTaps must be larger than 0");

    out.resize(numTaps);

    const int M = numTaps - 1;                 // filter order; center of the filter is at M / 2
    const double fc = cutoff / sampleRate;     // Hz -> cycles per sample

    double sum = 0;

    for (int i = 0; i < numTaps; ++i) {
        const double wi = Signal::Internal::Filters::windowValue(window, i, numTaps);

        out[i] = static_cast<float>(wi * Signal::Internal::Filters::sinc(i, M, fc));

        sum += out[i];
    }

    for (float& x : out) {
        x = static_cast<float>(x / sum);
    }
}

// sin(2*pi*fc*m) / m, with m = n - M/2 (offset from the center tap).
// Not divided by pi; the normalization in firwin makes that irrelevant.
double Signal::Internal::Filters::sinc(int n, int M, double cutoff) {
    const double m = n - M / 2.0;

    if (m == 0.0) return 2 * M_PI * cutoff;

    return std::sin(2 * M_PI * cutoff * m) / m;
}

double Signal::Internal::Filters::besselI0(double x, int iters) {
    const double y = x * x / 4.0;
    double sum = 1.0;       // k = 0 term
    double term = 1.0;

    for (int k = 1; k <= iters; ++k) {
        term *= y / (static_cast<double>(k) * k);
        sum += term;
        if (term < sum * 1e-17) break;
    }

    return sum;
}

float Signal::Internal::Filters::windowValue(filter_window_arg window, int n, int N) {
    assert(N > 0 && "N must be larger than 0");

    if (N == 1) return 1.0f;

    int M = N - 1;

    switch (window.windowType) {
    case WindowType::Rectangular:
        return 1.0f;
    case WindowType::Bartlett:
        return 1.0f - std::abs((2.0f * n / static_cast<float>(M)) - 1);
    case WindowType::Hann:
        return 0.5f - 0.5f * std::cos(2 * M_PI * n / M);
    case WindowType::Hamming:
        return 0.54f - 0.46f * std::cos(2 * M_PI * n / M);
    case WindowType::Blackman:
        return 0.42f - 0.5f * std::cos(2 * M_PI * n / M) + 0.08f * std::cos(4 * M_PI * n / M);
    case WindowType::BlackmanHarris: {
        constexpr double a0 = 0.35875;
        constexpr double a1 = 0.48829;
        constexpr double a2 = 0.14128;
        constexpr double a3 = 0.01168;

        return a0 - a1 * std::cos(2 * M_PI * n / M) + a2 * std::cos(4 * M_PI * n / M) - a3 * std::cos(6 * M_PI * n / M);
    }
    case WindowType::Kaiser: {
        float beta = window.beta;
        double args = beta * std::sqrt(1 - std::pow((2.0f * n / M) - 1, 2));

        double besselBeta = Signal::Internal::Filters::besselI0(beta);
        double besselArgs = Signal::Internal::Filters::besselI0(args);

        return static_cast<float>(besselArgs / besselBeta);   // I0(args) / I0(beta)
    }
    }

    return 1.0f;
}

// void Signal::General::shift_freq_to_zero_complex(
//     const std::complex<float>* data,
//     size_t size,
//     float offsetHz,
//     uint32_t sampleRate,
//     std::vector<std::complex<float>>& out
// ) {
//     out.resize(size);

//     std::complex<double> rotStep = std::polar(1.0, -2.0 * M_PI * offsetHz / sampleRate);
//     std::complex<double> rot{1.0, 0.0};

//     for (size_t i = 0; i < size; ++i) {
//         out[i] = data[i] * std::complex<float>(rot);

//         rot *= rotStep;
//     }
// }

template <typename T>
std::vector<T> Signal::General::decimate(const T* data, size_t size, size_t d) {
    std::vector<T> out(size / d);

    for (size_t i = 0, k = 0; i < size; i += d) out[k++] = data[i];

    return out;
}