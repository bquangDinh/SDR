#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <complex>
#include <fftw3.h>
#include <vector>

class Spectrum
{
public:
    Spectrum(size_t bins) : _bins(bins), avg(bins, 0.0f), window(bins, 0.0f) {
        in = fftwf_alloc_complex(bins);
        out = fftwf_alloc_complex(bins);

        plan = fftwf_plan_dft_1d(bins, in, out, FFTW_FORWARD, FFTW_MEASURE);

        make_blackman_window();
    }

    ~Spectrum() {
        fftwf_destroy_plan(plan);
        fftwf_free(in);
        fftwf_free(out);
    }

    void compute(const std::complex<float>* data, size_t n, std::vector<float>& out);

    // Use my own FFT implementation
    // Only for telemetry purpose
    void compute_fft_worse(const std::complex<float>* data, size_t n, std::vector<float>& out);
private:
    size_t _bins;
    std::vector<float> avg;
    std::vector<float> window;
    float _norm = 0;

    fftwf_plan plan;
    fftwf_complex* in;
    fftwf_complex* out;

    void recursive_fft(const std::complex<float>* x, size_t fft_bins, std::vector<std::complex<float>>& out);

    void make_blackman_window();
};

#endif // SPECTRUM_H
