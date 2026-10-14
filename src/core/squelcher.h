#ifndef SQUELCHER_H
#define SQUELCHER_H

#include <complex>

class Squelcher
{
public:
    Squelcher();

    void setSquelch(int squelch) { this->_squelch = squelch; }

    bool shouldSquelch(const std::complex<float>* data, size_t n);
private:
    int _squelch = 0;
};

#endif // SQUELCHER_H
