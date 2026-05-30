#include "rxn_hardware_monitor.h"
#include <chrono>
#include <cmath> // For sin and cos

RXNHardwareMonitor::RXNHardwareMonitor() : m_stopFlag(false) {}

RXNHardwareMonitor::~RXNHardwareMonitor() {
    Stop();
}

void RXNHardwareMonitor::Initialize() {
    // In a real implementation, this would initialize connections to
    // hardware monitoring APIs (e.g., NVML, PDH, etc.)
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
    snapshot.cpuUsage = m_atomicTelemetryData.cpuUsage.load(std::memory_order_relaxed);
    snapshot.gpuUsage = m_atomicTelemetryData.gpuUsage.load(std::memory_order_relaxed);
    snapshot.ramUsage = m_atomicTelemetryData.ramUsage.load(std::memory_order_relaxed);
    snapshot.gpuTemperature = m_atomicTelemetryData.gpuTemperature.load(std::memory_order_relaxed);
    snapshot.currentFps = m_atomicTelemetryData.currentFps.load(std::memory_order_relaxed);
    snapshot.averageFps = m_atomicTelemetryData.averageFps.load(std::memory_order_relaxed);
    snapshot.frameTime = m_atomicTelemetryData.frameTime.load(std::memory_order_relaxed);
    return snapshot;
}

void RXNHardwareMonitor::MonitorLoop() {
    while (!m_stopFlag) {
        // In a real implementation, poll hardware APIs here.
        // For now, simulate data changing.
        double time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count() / 1000.0;

        m_atomicTelemetryData.cpuUsage.store(50.0 + 10.0 * sin(time), std::memory_order_relaxed);
        m_atomicTelemetryData.gpuUsage.store(70.0 + 15.0 * cos(time), std::memory_order_relaxed);
        m_atomicTelemetryData.ramUsage.store(45.0 + 5.0 * sin(time * 0.5), std::memory_order_relaxed);
        m_atomicTelemetryData.gpuTemperature.store(65.0 + 5.0 * cos(time * 0.2), std::memory_order_relaxed);
        m_atomicTelemetryData.currentFps.store(144 + static_cast<int>(20 * sin(time * 2.0)), std::memory_order_relaxed);

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Poll every 100ms
    }
}
