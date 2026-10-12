#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

namespace Constants {
    namespace SampleRate {
        // Sample rates that are divisible by 48000 audio rate
        constexpr uint32_t RATE_0_96_MSPS = 960000;
        constexpr uint32_t RATE_1_2_MSPS = 1200000;
        constexpr uint32_t RATE_1_44_MSPS = 1440000;
        constexpr uint32_t RATE_1_92_MSPS = 1920000;
        constexpr uint32_t RATE_2_4_MSPS = 2400000;
        constexpr uint32_t RATE_2_88_MSPS = 2880000;
        constexpr uint32_t RATE_3_36_MSPS = 3360000;
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
