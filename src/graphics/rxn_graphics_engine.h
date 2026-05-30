#pragma once

#include <Windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

// Forward declarations to avoid circular dependencies and reduce compile times.
class RXNCapturePipeline;
class RXNConfig;

class RXNGraphicsEngine {
public:
    RXNGraphicsEngine();
    ~RXNGraphicsEngine();

    // Initializes the engine, its components (like the capture pipeline),
    // and links it to the application's central configuration.
    bool Initialize(HWND target_hwnd, RXNConfig* config_manager);

    // Starts the high-priority graphics thread.
    void Start();

    // Signals the graphics thread to terminate safely and waits for it to join.
    void Stop();

    // Dynamically sets the target frame rate for the graphics processing loop.
    void SetTargetFramerate(int fps);

private:
    // The main function for the graphics thread. This is where all the work happens.
    void GraphicsLoop();

    // --- Member Variables ---
    std::thread m_graphicsThread;
    std::atomic<bool> m_stopFlag;

    // A smart pointer to the capture pipeline. The engine owns this component.
    std::unique_ptr<RXNCapturePipeline> m_capturePipeline;
    
    // A non-owning pointer to the configuration object, which is owned by the main app.
    RXNConfig* m_configManager = nullptr;

    // Frame pacing variables to maintain a steady rhythm.
    std::chrono::nanoseconds m_targetFrameTime;

    HWND m_targetWindow = nullptr;
};
