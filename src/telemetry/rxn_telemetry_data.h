#pragma once

struct RXNTelemetryData {
    double cpuUsage = 0.0;
    double gpuUsage = 0.0;
    double ramUsage = 0.0;
    double gpuTemperature = 0.0;
    int currentFps = 0;
    int averageFps = 0;
    double frameTime = 0.0;

    RXNTelemetryData() = default;
    RXNTelemetryData(const RXNTelemetryData&) = default;
    RXNTelemetryData& operator=(const RXNTelemetryData&) = default;
};