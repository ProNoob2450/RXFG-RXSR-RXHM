#pragma once

#include <atomic>
#include <thread>

// A structure to hold all telemetry data. Using atomic members
// ensures that reads and writes from different threads are safe
// without requiring expensive mutexes.
struct RXNTelemetryData {
    std::atomic<double> frameTime;      // Placeholder for game frame time in ms
    std::atomic<int>    gpuTemperature; // Placeholder for GPU temp in Celsius
    std::atomic<int>    gpuUsagePercent;  // Placeholder for GPU utilization %
    std::atomic<int>    vramUsageMB;      // Placeholder for VRAM usage in megabytes
};

class RXNHardwareMonitor {
public:
    RXNHardwareMonitor();
    ~RXNHardwareMonitor();

    // Kicks off the background polling thread.
    void Start();

    // Signals the background thread to stop and waits for it to join.
    void Stop();

    // Provides safe, read-only access to the latest telemetry data.
    RXNTelemetryData GetLatestData() const;

private:
    // The main function for the background thread.
    void TelemetryLoop();

    // --- Member Variables ---
    std::atomic<bool> m_stopFlag;
    std::thread m_telemetryThread;
    RXNTelemetryData m_telemetryData;
};
