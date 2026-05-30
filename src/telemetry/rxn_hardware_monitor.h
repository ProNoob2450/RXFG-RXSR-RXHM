#pragma once

#include <thread>
#include <atomic>
#include "rxn_telemetry_data.h"

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

    // Safely retrieves the latest collected telemetry data.
    RXNTelemetryData GetLatestData() const;

private:
    // The background thread for polling hardware data.
    std::thread m_monitorThread;

    // Atomic flag to control the thread's lifecycle.
    std::atomic<bool> m_stopFlag;

    // The shared data structure for telemetry information.
    // Note: The struct itself uses atomic members for thread-safe updates.
    RXNTelemetryData m_telemetryData;

    // The main loop for the monitoring thread.
    void MonitorLoop();
};