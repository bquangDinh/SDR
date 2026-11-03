#include "telemetry.h"

#include <iostream>

void Telemetry::start_recording_deviceInitTime() {
    _startTime_deviceInitTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_deviceInitTime() {
    _endTime_deviceInitTime = std::chrono::steady_clock::now();

    _deviceInitTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_deviceInitTime - _startTime_deviceInitTime).count();
    
    _deviceInitTimeCount++;
}

void Telemetry::start_recording_deviceDestroyTime() {
    _startTime_deviceDestroyTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_deviceDestroyTime() {
    _endTime_deviceDestroyTime = std::chrono::steady_clock::now();

    _deviceDestroyTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_deviceDestroyTime - _startTime_deviceDestroyTime).count();
    
    _deviceDestroyTimeCount++;
}

void Telemetry::start_recording_readerThreadProcessingTime() {
    _startTime_readerThreadProcessingTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_readerThreadProcessingTime() {
    _endTime_readerThreadProcessingTime = std::chrono::steady_clock::now();

    _readerThreadProcessingTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_readerThreadProcessingTime - _startTime_readerThreadProcessingTime).count();
    
    _readerThreadProcessingTimeCount++;
}

void Telemetry::start_recording_deviceReadTime() {
    _startTime_deviceReadTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_deviceReadTime() {
    _endTime_deviceReadTime = std::chrono::steady_clock::now();

    _deviceReadTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_deviceReadTime - _startTime_deviceReadTime).count();
    
    _deviceReadTimeCount++;
}

void Telemetry::start_recording_chunkQueuePopTime() {
    _startTime_chunkQueuePopTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_chunkQueuePopTime() {
    _endTime_chunkQueuePopTime = std::chrono::steady_clock::now();

    _chunkQueuePopTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_chunkQueuePopTime - _startTime_chunkQueuePopTime).count();
    
    _chunkQueuePopTimeCount++;
}

void Telemetry::start_recording_chunkQueuePopImmediateTime() {
    _startTime_chunkQueuePopImmediateTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_chunkQueuePopImmediateTime() {
    _endTime_chunkQueuePopImmediateTime = std::chrono::steady_clock::now();

    _chunkQueuePopImmediateTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_chunkQueuePopImmediateTime - _startTime_chunkQueuePopImmediateTime).count();
    
    _chunkQueuePopImmediateTimeCount++;
}

void Telemetry::start_recording_AMdemodProcessingTime() {
    _startTime_AMdemodProcessingTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_AMdemodProcessingTime() {
    _endTime_AMdemodProcessingTime = std::chrono::steady_clock::now();

    _AMdemodProcessingTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_AMdemodProcessingTime - _startTime_AMdemodProcessingTime).count();
    
    _AMdemodProcessingTimeCount++;
}

void Telemetry::start_recording_WFMdemodProcessingTime() {
    _startTime_WFMdemodProcessingTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_WFMdemodProcessingTime() {
    _endTime_WFMdemodProcessingTime = std::chrono::steady_clock::now();

    _WFMdemodProcessingTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_WFMdemodProcessingTime - _startTime_WFMdemodProcessingTime).count();
    
    _WFMdemodProcessingTimeCount++;
}

void Telemetry::start_recording_NFMdemodProcessingTime() {
    _startTime_NFMdemodProcessingTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_NFMdemodProcessingTime() {
    _endTime_NFMdemodProcessingTime = std::chrono::steady_clock::now();

    _NFMdemodProcessingTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_NFMdemodProcessingTime - _startTime_NFMdemodProcessingTime).count();
    
    _NFMdemodProcessingTimeCount++;
}

void Telemetry::start_recording_FFTWideSpectrumProcessingTime() {
    _startTime_FFTWideSpectrumProcessingTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_FFTWideSpectrumProcessingTime() {
    _endTime_FFTWideSpectrumProcessingTime = std::chrono::steady_clock::now();

    _FFTWideSpectrumProcessingTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_FFTWideSpectrumProcessingTime - _startTime_FFTWideSpectrumProcessingTime).count();
    
    _FFTWideSpectrumProcessingTimeCount++;
}

void Telemetry::start_recording_FFTNarrowSpectrumProcessingTime() {
    _startTime_FFTNarrowSpectrumProcessingTime = std::chrono::steady_clock::now();
}

void Telemetry::stop_recording_FFTNarrowSpectrumProcessingTime() {
    _endTime_FFTNarrowSpectrumProcessingTime = std::chrono::steady_clock::now();

    _FFTNarrowSpectrumProcessingTime += std::chrono::duration_cast<std::chrono::microseconds>(_endTime_FFTNarrowSpectrumProcessingTime - _startTime_FFTNarrowSpectrumProcessingTime).count();
    
    _FFTNarrowSpectrumProcessingTimeCount++;
}

void Telemetry::print_report() {
    // Compute averages
    double readerThreadProcessingTimeAvg = _readerThreadProcessingTimeCount > 0 ? static_cast<double>(_readerThreadProcessingTime) / _readerThreadProcessingTimeCount : 0.0;
    double AMdemodProcessingTimeAvg = _AMdemodProcessingTimeCount > 0 ? static_cast<double>(_AMdemodProcessingTime) / _AMdemodProcessingTimeCount : 0.0;
    double WFMdemodProcessingTimeAvg = _WFMdemodProcessingTimeCount > 0 ? static_cast<double>(_WFMdemodProcessingTime) / _WFMdemodProcessingTimeCount : 0.0;
    double NFMdemodProcessingTimeAvg = _NFMdemodProcessingTimeCount > 0 ? static_cast<double>(_NFMdemodProcessingTime) / _NFMdemodProcessingTimeCount : 0.0;
    double chunkQueuePopTimeAvg = _chunkQueuePopTimeCount > 0 ? static_cast<double>(_chunkQueuePopTime) / _chunkQueuePopTimeCount : 0.0;
    double chunkQueuePopImmediateTimeAvg = _chunkQueuePopImmediateTimeCount > 0 ? static_cast<double>(_chunkQueuePopImmediateTime) / _chunkQueuePopImmediateTimeCount : 0.0;
    double FFTWideSpectrumProcessingTimeAvg = _FFTWideSpectrumProcessingTimeCount > 0 ? static_cast<double>(_FFTWideSpectrumProcessingTime) / _FFTWideSpectrumProcessingTimeCount : 0.0;
    double FFTNarrowSpectrumProcessingTimeAvg = _FFTNarrowSpectrumProcessingTimeCount > 0 ? static_cast<double>(_FFTNarrowSpectrumProcessingTime) / _FFTNarrowSpectrumProcessingTimeCount : 0.0;
    double deviceInitTimeAvg = _deviceInitTimeCount > 0 ? static_cast<double>(_deviceInitTime) / _deviceInitTimeCount : 0.0;
    double deviceReadTimeAvg = _deviceReadTimeCount > 0 ? static_cast<double>(_deviceReadTime) / _deviceReadTimeCount : 0.0;
    double deviceDestroyTimeAvg = _deviceDestroyTimeCount > 0 ? static_cast<double>(_deviceDestroyTime) / _deviceDestroyTimeCount : 0.0;

    std::cout << "-------------------Telemetry Report---------------------" << std::endl;
    std::cout << "Device Init Time Avg: " << deviceInitTimeAvg << " us" << std::endl;
    std::cout << "Device Read Time Avg: " << deviceReadTimeAvg << " us" << std::endl;
    std::cout << "Device Destroy Time Avg: " << deviceDestroyTimeAvg << " us" << std::endl;
    std::cout << "Reader Thread Processing Time Avg: " << readerThreadProcessingTimeAvg << " us" << std::endl;
    std::cout << "AM Demod Processing Time Avg: " << AMdemodProcessingTimeAvg << " us" << std::endl;
    std::cout << "WFM Demod Processing Time Avg: " << WFMdemodProcessingTimeAvg << " us" << std::endl;
    std::cout << "NFM Demod Processing Time Avg: " << NFMdemodProcessingTimeAvg << " us" << std::endl;
    std::cout << "Chunk Queue Pop Time Avg: " << chunkQueuePopTimeAvg << " us" << std::endl;
    std::cout << "Chunk Queue Pop Immediate Time Avg: " << chunkQueuePopImmediateTimeAvg << " us" << std::endl;
    std::cout << "FFT Wide Spectrum Processing Time Avg: " << FFTWideSpectrumProcessingTimeAvg << " us" << std::endl;
    std::cout << "FFT Narrow Spectrum Processing Time Avg: " << FFTNarrowSpectrumProcessingTimeAvg << " us" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
}