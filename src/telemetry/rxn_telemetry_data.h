#pragma once

#include <atomic>

/**
 * @struct RXNTelemetryData
 * @brief A plain data structure to hold a snapshot of telemetry data.
 * 
 * This struct contains non-atomic members and is used to safely transfer
 * telemetry data across thread boundaries. The RXNHardwareMonitor is responsible
 * for creating instances of this struct in a thread-safe manner.
 */
struct RXNTelemetryData {
    // --- System Metrics ---
    double cpuUsage = 0.0;
    double gpuUsage = 0.0;
    double ramUsage = 0.0;
    double gpuTemperature = 0.0;

    // --- Performance Metrics ---
    int currentFps = 0;
    int averageFps = 0;
    double frameTime = 0.0;

    // Default constructor
    RXNTelemetryData() = default;

    // Explicitly define copy and move constructors/assignments for clarity
    RXNTelemetryData(const RXNTelemetryData&) = default;
    RXNTelemetryData& operator=(const RXNTelemetryData&) = default;
    RXNTelemetryData(RXNTelemetryData&&) = default;
    RXNTelemetryData& operator=(RXNTelemetryData&&) = default;
};