#include "rxn_hardware_monitor.h"
#include <chrono>

RXNHardwareMonitor::RXNHardwareMonitor() : m_stopFlag(false) {}

RXNHardwareMonitor::~RXNHardwareMonitor() {
    Stop();
}

void RXNHardwareMonitor::Initialize() {
    // In a real implementation, this would initialize connections to
    // hardware monitoring APIs (e.g., NVML, WMI, etc.)
}

void RXNHardwareMonitor::Start() {
    if (!m_monitorThread.joinable()) {
        m_stopFlag = false;
        m_monitorThread = std::thread(&RXNHardwareMonitor::MonitorLoop, this);
    }
}

void RXNHardwareMonitor::Stop() {
    if (m_monitorThread.joinable()) {
        m_stopFlag = true;
        m_monitorThread.join();
    }
}

// Safely retrieves the latest collected telemetry data.
RXNTelemetryData RXNHardwareMonitor::GetLatestData() const {
    RXNTelemetryData snapshot;
    // Atomically load the values from the internal atomic members.
    // Note: This assumes m_telemetryData is now a struct of atomics,
    // or the loads are handled within MonitorLoop.
    snapshot.cpuUsage = m_telemetryData.cpuUsage.load();
    snapshot.gpuUsage = m_telemetryData.gpuUsage.load();
    snapshot.ramUsage = m_telemetryData.ramUsage.load();
    snapshot.gpuTemperature = m_telemetryData.gpuTemperature.load();
    snapshot.currentFps = m_telemetryData.currentFps.load();
    snapshot.averageFps = m_telemetryData.averageFps.load();
    snapshot.frameTime = m_telemetryData.frameTime.load();
    return snapshot;
}

void RXNHardwareMonitor::MonitorLoop() {
    while (!m_stopFlag) {
        // In a real implementation, poll hardware APIs here.
        // For now, simulate data changing.
        double time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count() / 1000.0;

        m_telemetryData.cpuUsage.store(50.0 + 10.0 * sin(time));
        m_telemetryData.gpuUsage.store(70.0 + 15.0 * cos(time));
        m_telemetryData.ramUsage.store(45.0 + 5.0 * sin(time * 0.5));
        m_telemetryData.gpuTemperature.store(65.0 + 5.0 * cos(time * 0.2));
        m_telemetryData.currentFps.store(144 + static_cast<int>(20 * sin(time * 2.0)));

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Poll every 100ms
    }
}
