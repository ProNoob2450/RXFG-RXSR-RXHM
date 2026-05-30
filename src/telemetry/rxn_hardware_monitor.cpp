#include "rxn_hardware_monitor.h"
#include <chrono> // For std::chrono

RXNHardwareMonitor::RXNHardwareMonitor() : m_stopFlag(false) {}

RXNHardwareMonitor::~RXNHardwareMonitor() {
    Stop();
}

void RXNHardwareMonitor::Initialize() {
    // In a real-world scenario, this is where you'd initialize your hardware
    // monitoring libraries (e.g., NVML for NVIDIA, AGS for AMD, or PDH for Windows).
    // For this example, we'll keep it simple.
}

void RXNHardwareMonitor::Start() {
    if (m_monitorThread.joinable()) {
        return; // Already running
    }
    m_stopFlag = false;
    m_monitorThread = std::thread(&RXNHardwareMonitor::MonitorLoop, this);
}

void RXNHardwareMonitor::Stop() {
    if (m_monitorThread.joinable()) {
        m_stopFlag = true;
        m_monitorThread.join();
    }
}

RXNTelemetryData RXNHardwareMonitor::GetLatestData() const {
    RXNTelemetryData snapshot;
    // Atomically load the current values into the snapshot struct.
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
        // --- Placeholder Data Collection ---
        // In a real implementation, you would query hardware APIs here.
        // We'll simulate some data changes for demonstration.
        double simulated_cpu_usage = 50.0 + (rand() % 10 - 5); // 45-55%
        double simulated_gpu_usage = 70.0 + (rand() % 10 - 5); // 65-75%
        double simulated_ram_usage = 40.0 + (rand() % 5);       // 40-45%
        double simulated_gpu_temp = 75.0 + (rand() % 5 - 2);     // 73-78 C

        // Atomically store the new values.
        m_atomicTelemetryData.cpuUsage.store(simulated_cpu_usage, std::memory_order_relaxed);
        m_atomicTelemetryData.gpuUsage.store(simulated_gpu_usage, std::memory_order_relaxed);
        m_atomicTelemetryData.ramUsage.store(simulated_ram_usage, std::memory_order_relaxed);
        m_atomicTelemetryData.gpuTemperature.store(simulated_gpu_temp, std::memory_order_relaxed);
        
        // The FPS and frame time would likely be updated from the graphics engine,
        // but we simulate it here for completeness.
        int simulated_fps = 58 + (rand() % 5); // 58-62 FPS
        m_atomicTelemetryData.currentFps.store(simulated_fps, std::memory_order_relaxed);
        m_atomicTelemetryData.frameTime.store(1000.0 / simulated_fps, std::memory_order_relaxed);

        // Sleep for a defined interval (e.g., 1 second) to avoid busy-waiting.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
