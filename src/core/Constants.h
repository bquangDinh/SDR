#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

namespace Constants {
    namespace SampleRate {
        constexpr uint32_t RATE_3_2_MSPS   = 3200000;
        constexpr uint32_t RATE_2_8_MSPS   = 2800000;
        constexpr uint32_t RATE_2_56_MSPS  = 2560000;
        constexpr uint32_t RATE_2_4_MSPS   = 2400000;
        constexpr uint32_t RATE_2_048_MSPS = 2048000;
        constexpr uint32_t RATE_1_92_MSPS  = 1920000;
        constexpr uint32_t RATE_1_8_MSPS   = 1800000;
        constexpr uint32_t RATE_1_4_MSPS   = 1400000;
        constexpr uint32_t RATE_1_024_MSPS = 1024000;
        constexpr uint32_t RATE_0_900001_MSPS = 900001;
        constexpr uint32_t RATE_0_25_MSPS  = 250000;
    }

    namespace DemodulateMode {
        constexpr int NFM = 0;
        constexpr int AM = 1;
        constexpr int LSB = 2;
        constexpr int USB = 3;
        constexpr int WFM = 4;
        constexpr int DSB = 5;
        constexpr int CW = 6;
        constexpr int RAW = 7;
    }
}

#endif // CONSTANTS_H
