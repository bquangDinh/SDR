#include "core/rtl_thread.h"

#include <vector>
#include <cmath>

void RtlThread::setCenterFreq(quint32 freq) {
    centerFreq_ = freq;

    sdr_set_center_freq(freq);
}

void RtlThread::setRfGain(int rfGain) {
    rfGain_ = rfGain;

    sdr_set_rf_gain(rfGain);
}

void RtlThread::sample(const char *buf, int n, int binsCount, std::complex<float> *outBuf) {
    const int pairs = n / 2;
    if (pairs < binsCount) return;

    const auto *u = reinterpret_cast<const unsigned char *>(buf); // 0 - 255

    std::vector<std::complex<float>> s(binsCount);
    for (int i = 0; i < binsCount; ++i)
        s[i] = { (u[2 * i]     - 127.5f) / 127.5f,
                (u[2 * i + 1] - 127.5f) / 127.5f }; // -1.0f to 1.0f

    // compute e_term once
    std::vector<std::complex<float>> tw(binsCount);
    for (int m = 0; m < binsCount; ++m)
        tw[m] = std::polar(1.0f, -2.0f * float(M_PI) * m / binsCount);

    for (int k = 0; k < binsCount; ++k) {
        std::complex<float> sum = 0;
        for (int i = 0; i < binsCount; ++i)
            sum += s[i] * tw[(long long)k * i % binsCount];   // angle wraps every full turn
        outBuf[k] = sum;
    }
}

void RtlThread::toDBFS(const std::complex<float>* X, int binsCount, std::vector<float> &out) {
    for (int k = 0; k < binsCount; ++k) {
        int src = (k + binsCount / 2) % binsCount;

        float mag = std::abs(X[src]) / binsCount; // normalized

        out[k] = 20.0f * std::log10(mag + 1e-20f); // avoid log(10)(0)
    }
}
