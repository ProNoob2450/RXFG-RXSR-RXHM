#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include "rxn_telemetry_data.h"

// Forward declaration to hide implementation details
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

    // Initializes the hardware monitor.
    void Initialize();

    // Starts the monitoring thread.
    void Start();

    // Stops the monitoring thread.
    void Stop();

    /**
     * @brief Safely retrieves the latest collected telemetry data.
     * @return A snapshot of the telemetry data.
     */
    RXNTelemetryData GetLatestData() const;

private:
    // The internal monitoring loop that runs on a separate thread.
    void MonitorLoop();

    std::thread m_monitorThread;
    std::atomic<bool> m_stopFlag;

    // Holds the latest telemetry data, updated by the monitor thread.
    // Using the Pimpl idiom or a nested struct is a good practice to
    // keep the header clean and compilation times fast.
    mutable RXNAtomicTelemetryData m_atomicTelemetryData;
};
