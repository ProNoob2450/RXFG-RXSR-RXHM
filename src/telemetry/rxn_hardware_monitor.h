#pragma once

#include <thread>
#include <atomic>
#include "rxn_telemetry_data.h" // Plain data struct

// Internal struct with atomic members for thread-safe updates
struct RXNAtomicTelemetryData {
    std::atomic<double> cpuUsage{0.0};
    std::atomic<double> gpuUsage{0.0};
    std::atomic<double> ramUsage{0.0};
    std::atomic<double> gpuTemperature{0.0};
    std::atomic<int>   currentFps{0};
    std::atomic<int>   averageFps{0};
    std::atomic<double> frameTime{0.0};
};

class RXNHardwareMonitor {
public:
    RXNHardwareMonitor();
    ~RXNHardwareMonitor();

    // Initializes the hardware monitor.
    void Initialize();

    // Starts the background monitoring thread.
    void Start();

    // Stops the background monitoring thread.
    void Stop();

    // Safely retrieves the latest collected telemetry data as a plain struct.
    RXNTelemetryData GetLatestData() const;

private:
    // The background thread for polling hardware data.
    std::thread m_monitorThread;

    // Atomic flag to control the thread's lifecycle.
    std::atomic<bool> m_stopFlag;

    // The internal, thread-safe data structure with atomic members.
    RXNAtomicTelemetryData m_atomicTelemetryData;

    // The main loop for the monitoring thread.
    void MonitorLoop();
};