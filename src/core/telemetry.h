#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "core/singleton.h"
#include <chrono>

class Telemetry : public Singleton<Telemetry>
{
    friend class Singleton<Telemetry>;
private:
    Telemetry() = default;

    ~Telemetry() = default;

    uint64_t _deviceInitTime;
    uint64_t _deviceInitTimeCount;
    std::chrono::steady_clock::time_point _startTime_deviceInitTime;
    std::chrono::steady_clock::time_point _endTime_deviceInitTime;

    uint64_t _deviceReadTime;
    uint64_t _deviceReadTimeCount;
    std::chrono::steady_clock::time_point _startTime_deviceReadTime;
    std::chrono::steady_clock::time_point _endTime_deviceReadTime;

    uint64_t _deviceDestroyTime;
    uint64_t _deviceDestroyTimeCount;
    std::chrono::steady_clock::time_point _startTime_deviceDestroyTime;
    std::chrono::steady_clock::time_point _endTime_deviceDestroyTime;

    uint64_t _readerThreadProcessingTime;
    uint64_t _readerThreadProcessingTimeCount;
    std::chrono::steady_clock::time_point _startTime_readerThreadProcessingTime;
    std::chrono::steady_clock::time_point _endTime_readerThreadProcessingTime;

    uint64_t _AMdemodProcessingTime;
    uint64_t _AMdemodProcessingTimeCount;
    std::chrono::steady_clock::time_point _startTime_AMdemodProcessingTime;
    std::chrono::steady_clock::time_point _endTime_AMdemodProcessingTime;

    uint64_t _NFMdemodProcessingTime;
    uint64_t _NFMdemodProcessingTimeCount;
    std::chrono::steady_clock::time_point _startTime_NFMdemodProcessingTime;
    std::chrono::steady_clock::time_point _endTime_NFMdemodProcessingTime;

    uint64_t _WFMdemodProcessingTime;
    uint64_t _WFMdemodProcessingTimeCount;
    std::chrono::steady_clock::time_point _startTime_WFMdemodProcessingTime;
    std::chrono::steady_clock::time_point _endTime_WFMdemodProcessingTime;

    uint64_t _chunkSize;
    uint64_t _chunkSizeCount;

    uint64_t _chunkQueuePopTime;
    uint64_t _chunkQueuePopTimeCount;
    std::chrono::steady_clock::time_point _startTime_chunkQueuePopTime;
    std::chrono::steady_clock::time_point _endTime_chunkQueuePopTime;

    uint64_t _chunkQueuePopImmediateTime;
    uint64_t _chunkQueuePopImmediateTimeCount;
    std::chrono::steady_clock::time_point _startTime_chunkQueuePopImmediateTime;
    std::chrono::steady_clock::time_point _endTime_chunkQueuePopImmediateTime;

    uint64_t _FFTWideSpectrumProcessingTime;
    uint64_t _FFTWideSpectrumProcessingTimeCount;
    std::chrono::steady_clock::time_point _startTime_FFTWideSpectrumProcessingTime;
    std::chrono::steady_clock::time_point _endTime_FFTWideSpectrumProcessingTime;
    
    uint64_t _FFTNarrowSpectrumProcessingTime;
    uint64_t _FFTNarrowSpectrumProcessingTimeCount;
    std::chrono::steady_clock::time_point _startTime_FFTNarrowSpectrumProcessingTime;
    std::chrono::steady_clock::time_point _endTime_FFTNarrowSpectrumProcessingTime;

public:
    void start_recording_deviceInitTime();
    void stop_recording_deviceInitTime();

    void start_recording_deviceReadTime();
    void stop_recording_deviceReadTime();

    void start_recording_deviceDestroyTime();
    void stop_recording_deviceDestroyTime();

    void start_recording_readerThreadProcessingTime();
    void stop_recording_readerThreadProcessingTime();

    void start_recording_AMdemodProcessingTime();
    void stop_recording_AMdemodProcessingTime();

    void start_recording_WFMdemodProcessingTime();
    void stop_recording_WFMdemodProcessingTime();

    void start_recording_NFMdemodProcessingTime();
    void stop_recording_NFMdemodProcessingTime();

    void start_recording_FFTWideSpectrumProcessingTime();
    void stop_recording_FFTWideSpectrumProcessingTime();

    void start_recording_FFTNarrowSpectrumProcessingTime();
    void stop_recording_FFTNarrowSpectrumProcessingTime();

    void start_recording_chunkQueuePopTime();
    void stop_recording_chunkQueuePopTime();

    void start_recording_chunkQueuePopImmediateTime();
    void stop_recording_chunkQueuePopImmediateTime();

    void print_report();
};

#endif // TELEMETRY_H
