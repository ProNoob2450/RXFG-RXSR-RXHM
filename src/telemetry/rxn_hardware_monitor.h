#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include "rxn_telemetry_data.h"

struct RXNAtomicTelemetryData {
    std::atomic<double> cpuUsage{0.0};
    std::atomic<double> gpuUsage{0.0};
    std::atomic<double> ramUsage{0.0};
    std::atomic<double> gpuTemperature{0.0};
    std::atomic<int> currentFps{0};
    std::atomic<int> averageFps{0};
    std::atomic<double> frameTime{0.0};
};

class RXNHardwareMonitor {
public:
    RXNHardwareMonitor();
    ~RXNHardwareMonitor();
    
    void Initialize();
    void Start();
    void Stop();
    RXNTelemetryData GetLatestData() const;

private:
    void MonitorLoop();
    std::thread m_monitorThread;
    std::atomic<bool> m_stopFlag{false};
    mutable RXNAtomicTelemetryData m_atomicTelemetryData;
};