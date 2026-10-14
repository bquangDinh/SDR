#include "squelcher.h"

#include <QDebug>

Squelcher::Squelcher() {}

bool Squelcher::shouldSquelch(const std::complex<float>* data, size_t n) {
    double avg = 0;

    for (size_t i = 0; i < n; ++i) {
        avg += std::norm(data[i]);
    }

    avg /= n;

    avg = std::abs(10.0 * std::log10(avg));

    qDebug() << "Average power is: " << avg << " vs " << _squelch;

    if (avg > _squelch) return true;

    return false;
}