#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

struct service_band {
    uint32_t lower_freq;
    uint32_t upper_freq;
    const char* name;
};

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
        constexpr int NONE = 8;
    }

    namespace SamplingMode {
        constexpr int QUADRATURE_SAMPLING = 0;
        constexpr int DIRECT_SAMPLING_I_BRANCH = 1;
        constexpr int DIRECT_SAMPLING_Q_BRANCH = 2;
    }

    namespace ServiceBand {
        constexpr service_band BANDS[] = {
            // HF
            {190000, 535000, "Aircraft navigation beacons (NDB) - AM/Morse"},
            {530000, 1700000, "AM Broadcast - AM"},
            {1800000, 2000000, "Ham 160m - SSB/CW/digital"},
            {2300000, 2500000, "Shortwave Broadcast - AM"},
            {2500000, 2500000, "Time signals (WWV) 2.5 MHz - AM"},
            {3500000, 4000000, "Ham 80m - SSB/CW/digital"},
            {5000000, 5000000, "Time signals (WWV) 5 MHz - AM"},
            {5900000, 6200000, "Shortwave Broadcast 49m - AM"},
            {7000000, 7300000, "Ham 40m - SSB/CW/digital"},
            {7200000, 7450000, "Shortwave Broadcast 41m - AM"},
            {9400000, 9900000, "Shortwave Broadcast 31m - AM"},
            {10000000, 10000000, "Time signals (WWV) 10 MHz - AM"},
            {11600000, 12100000, "Shortwave Broadcast 25m - AM"},
            {14000000, 14350000, "Ham 20m - SSB/CW/digital"},
            {15000000, 15000000, "Time signals (WWV) 15 MHz - AM"},
            {20000000, 20000000, "Time signals (WWV) 20 MHz - AM"},
            {21000000, 21450000, "Ham 15m - SSB/CW/digital"},
            {26100000, 26100000, "Shortwave Broadcast (upper edge) - AM"},
            {26965000, 27405000, "CB Radio - AM"},
            {28000000, 29700000, "Ham 10m - SSB/CW/digital"},

            // VHF
            {50000000, 54000000, "Ham 6m - SSB/FM"},
            {54000000, 88000000, "TV low channels 2-6 - Digital (ATSC)"},
            {88000000, 108000000, "FM Broadcast - Wide FM"},
            {108000000, 118000000, "Aircraft navigation (VOR/ILS) - AM tones"},
            {118000000, 137000000, "Airband (voice, ATIS) - AM"},
            {137000000, 138000000, "Weather satellites (NOAA APT, Meteor) - FM/digital"},
            {144000000, 148000000, "Ham 2m - NFM"},
            {150000000, 174000000, "Land Mobile / Public Safety (VHF high) - NFM/digital"},
            {151820000, 154600000, "MURS - NFM"},
            {152000000, 152000000, "Pagers (~152 MHz) - POCSAG"},
            {157000000, 157000000, "Pagers (~157 MHz) - POCSAG"},
            {156000000, 162025000, "VHF Marine Band - NFM"},
            {162400000, 162550000, "NOAA Weather Radio - NFM"},
            {174000000, 216000000, "TV high channels 7-13 - Digital"},
            {222000000, 225000000, "Ham 1.25m - NFM"},

            // UHF and above
            {225000000, 380000000, "Military Airband - AM"},
            {420000000, 450000000, "Ham 70cm - NFM"},
            {433920000, 433920000, "Low-power devices (key fobs, sensors) - OOK/FSK"},
            {450000000, 470000000, "Land Mobile / Public Safety (UHF) - NFM/digital"},
            {462562500, 467712500, "FRS / GMRS - NFM"},
            {470000000, 608000000, "TV UHF channels 14-36 - Digital"},
            {617000000, 894000000, "Cell phones (600/700/850 MHz) - LTE/5G"},
            {806000000, 824000000, "800 MHz trunked public safety - P25"},
            {851000000, 869000000, "800 MHz trunked public safety - P25"},
            {902000000, 928000000, "ISM band (smart meters, LoRa) - Digital"},
            {929000000, 932000000, "Pagers - POCSAG"},
            {1090000000, 1090000000, "ADS-B (aircraft position) - Digital pulses"},
            {1240000000, 1300000000, "Ham 23cm - various"},
            {1525000000, 1559000000, "Satellite phone/data (Inmarsat) - Digital"},
            {1575420000, 1575420000, "GPS L1 - Digital (spread spectrum)"},
        };
    }
}

#endif // CONSTANTS_H
