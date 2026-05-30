#include "rxn_hardware_monitor.h"
#include <chrono>

RXNHardwareMonitor::RXNHardwareMonitor() {
    m_stopFlag.store(false);
    // Initialize atomics with default values
    m_telemetryData.frameTime.store(0.0);
    m_telemetryData.gpuTemperature.store(0);
    m_telemetryData.gpuUsagePercent.store(0);
    m_telemetryData.vramUsageMB.store(0);
}

RXNHardwareMonitor::~RXNHardwareMonitor() {
    Stop(); // Ensure thread is stopped and joined on destruction
}

void RXNHardwareMonitor::Start() {
    if (m_telemetryThread.joinable()) {
        return; // Already running
    }
    m_stopFlag.store(false);
    m_telemetryThread = std::thread(&RXNHardwareMonitor::TelemetryLoop, this);
}

void RXNHardwareMonitor::Stop() {
    m_stopFlag.store(true);
    if (m_telemetryThread.joinable()) {
        m_telemetryThread.join();
    }
}

RXNTelemetryData RXNHardwareMonitor::GetLatestData() const {
    // The structure itself is copied, but the atomic members guarantee
    // that each individual read is safe.
    RXNTelemetryData snapshot;
    snapshot.frameTime.store(m_telemetryData.frameTime.load());
    snapshot.gpuTemperature.store(m_telemetryData.gpuTemperature.load());
    snapshot.gpuUsagePercent.store(m_telemetryData.gpuUsagePercent.load());
    snapshot.vramUsageMB.store(m_telemetryData.vramUsageMB.load());
    return snapshot;
}

void RXNHardwareMonitor::TelemetryLoop() {
    while (!m_stopFlag.load()) {
        // --- Placeholder Polling Logic ---
        // In a real implementation, this is where you would query NVAPI, ADL, or WMI.
        // For now, we simulate data fluctuations.

        // Simulate frame time (e.g., oscillating between 15ms and 17ms)
        double newFrameTime = 16.6 + (rand() % 200) / 100.0 - 1.0;
        m_telemetryData.frameTime.store(newFrameTime);

        // Simulate GPU temperature (e.g., 65-75C)
        int newGpuTemp = 65 + (rand() % 11);
        m_telemetryData.gpuTemperature.store(newGpuTemp);

        // Simulate GPU usage (e.g., 80-99%)
        int newGpuUsage = 80 + (rand() % 20);
        m_telemetryData.gpuUsagePercent.store(newGpuUsage);

        // Simulate VRAM usage (e.g., 4096-6144 MB)
        int newVramUsage = 4096 + (rand() % 2049);
        m_telemetryData.vramUsageMB.store(newVramUsage);

        // --- End Placeholder Logic ---

        // Sleep for 500ms to meet the polling cycle requirement.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
