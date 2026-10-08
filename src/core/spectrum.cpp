#include "spectrum.h"

void Spectrum::compute(const std::complex<float>* data, size_t n, std::vector<float>& output) {
    const int blocks = n / _bins;

    if (blocks == 0) return;

    auto* x = reinterpret_cast<std::complex<float>*>(in);
    auto* X = reinterpret_cast<std::complex<float>*>(out);

    std::fill(avg.begin(), avg.end(), 0.0f);

    output.resize(_bins);

    for (int b = 0; b < blocks; ++b) {
        const std::complex<float>* src = data + b * _bins;

        // Fill in
        for (int k = 0; k < _bins; ++k) {
            x[k] = src[k] * window[k];
        }

        // Compute
        fftwf_execute(plan);

        for (int k = 0; k < _bins; ++k) {
            avg[k] += std::norm(X[k]);
        }
    }

    for (int k = 0; k < _bins; ++k) {
        // Since the layout of X is
        // 0 -> 1 -------------> (N / 2) - 1 -> N / 2 ------------> N - 1
        // 0 -> [positive -> most positive] -> [most negative -> negative]
        // we want the output bin to be the following layout:
        // [most negative -> least negative] 0 [least positive -> most positive]
        const int s = (k + _bins / 2) % _bins;

        output[k] = 10.0f * std::log10((avg[s] / blocks) * _norm + 1e-20f);
    }
}

void Spectrum::make_blackman_window() {
    double sum = 0;

    for (int i = 0; i < _bins; ++i) {
        window[i] = float(0.42f - 0.5f * std::cos(2 * M_PI * i / (_bins - 1))) + 0.08 * std::cos(4 * M_PI * i / (_bins - 1));

        sum += window[i];
    }

    _norm = float(1.0f / (sum * sum));
}

void Spectrum::compute_fft_worse(const std::complex<float>* data, size_t n, std::vector<float>& output) {
    const int blocks = n / _bins;

    std::vector<std::complex<float>> in(n);

    std::vector<std::complex<float>> outFFT(n);

    if (blocks == 0) return;

    std::fill(avg.begin(), avg.end(), 0.0f);

    output.resize(_bins);

    for (int b = 0; b < blocks; ++b) {
        const std::complex<float>* src = data + b * _bins;

        for (int k = 0; k < _bins; ++k) {
            in[k] = src[k] * window[k];
        }

        recursive_fft(in.data(), _bins, outFFT);

        for (int k = 0; k < _bins; ++k) {
            // Take square distance, kind of faster than std::abs since there is no square root
            avg[k] += std::norm(outFFT[k]);
        }
    }

    for (int k = 0; k < _bins; ++k) {
        // Since the layout of X is
        // 0 -> 1 -------------> (N / 2) - 1 -> N / 2 ------------> N - 1
        // 0 -> [positive -> most positive] -> [most negative -> negative]
        // we want the output bin to be the following layout:
        // [most negative -> least negative] 0 [least positive -> most positive]
        const int s = (k + _bins / 2) % _bins;

        output[k] = 10.0f * std::log10((avg[s] / blocks) * _norm + 1e-20f);
    }
}

void Spectrum::recursive_fft(const std::complex<float>* x, size_t fft_bins, std::vector<std::complex<float>>& out) {
    // TODO: check if fft_bins is the power of 2
    // TODO: check boundary

    if (fft_bins <= 1) {
        /// FFT of one element is the element itself
        out[0] = x[0];

        return;
    }

    std::vector<std::complex<float>> even_terms(fft_bins / 2);
    std::vector<std::complex<float>> odd_terms(fft_bins / 2);

    for (int i = 0; i < fft_bins / 2; ++i) {
        even_terms[i] = x[2 * i];
        odd_terms[i] = x[2* i + 1];
    }

    std::vector<std::complex<float>> E(fft_bins / 2);
    std::vector<std::complex<float>> O(fft_bins / 2);

    recursive_fft(even_terms.data(), fft_bins / 2, E);
    recursive_fft(odd_terms.data(), fft_bins / 2, O);

    std::complex<float> W;

    for (int i = 0; i < fft_bins / 2; ++i) {
        W = std::polar(1.0, -2.0f * M_PI * i / fft_bins);

        out[i] = E[i] + W * O[i];
        out[i + fft_bins / 2] = E[i] - W * O[i];
    }
}

