#include "rxn_graphics_engine.h"
#include "../main/rxn_config.h"       // To access settings
#include "rxn_capture_pipeline.h" // To access the capture API

RXNGraphicsEngine::RXNGraphicsEngine() 
    : m_stopFlag(false), m_configManager(nullptr), m_targetWindow(nullptr) {
    // Default to 60 FPS until set otherwise.
    SetTargetFramerate(60);
}

RXNGraphicsEngine::~RXNGraphicsEngine() {
    Stop();
}

bool RXNGraphicsEngine::Initialize(HWND target_hwnd, RXNConfig* config_manager) {
    if (!config_manager) {
        return false;
    }
    m_configManager = config_manager;
    m_targetWindow = target_hwnd;

    // The graphics engine takes ownership of the capture pipeline.
    m_capturePipeline = std::make_unique<RXNCapturePipeline>();
    HRESULT hr = m_capturePipeline->Initialize(m_targetWindow);
    
    return SUCCEEDED(hr);
}

void RXNGraphicsEngine::Start() {
    if (m_graphicsThread.joinable()) {
        return; // Thread is already running.
    }

    m_stopFlag.store(false);
    m_graphicsThread = std::thread(&RXNGraphicsEngine::GraphicsLoop, this);

    // Elevate the thread priority to ensure it is not preempted by other system tasks,
    // which is critical for smooth frame pacing.
    if (m_graphicsThread.native_handle()) {
        SetThreadPriority(m_graphicsThread.native_handle(), THREAD_PRIORITY_HIGHEST); // REALTIME_PRIORITY requires admin
    }
}

void RXNGraphicsEngine::Stop() {
    m_stopFlag.store(true);
    if (m_graphicsThread.joinable()) {
        m_graphicsThread.join();
    }
}

void RXNGraphicsEngine::SetTargetFramerate(int fps) {
    if (fps > 0) {
        m_targetFrameTime = std::chrono::nanoseconds(1000000000 / fps);
    } else {
        // A value of 0 or less indicates an unlocked framerate.
        m_targetFrameTime = std::chrono::nanoseconds(0);
    }
}

void RXNGraphicsEngine::GraphicsLoop() {
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (!m_stopFlag.load()) {
        auto startOfFrame = std::chrono::high_resolution_clock::now();
        auto elapsedSinceLastFrame = startOfFrame - lastFrameTime;

        // Enforce frame pacing if a target frame rate is set.
        if (m_targetFrameTime.count() > 0 && elapsedSinceLastFrame < m_targetFrameTime) {
            // Simple sleep to yield CPU time. More advanced implementations might use
            // a busy-wait loop for higher precision, but this is less resource-intensive.
            std::this_thread::sleep_for(m_targetFrameTime - elapsedSinceLastFrame);
        }
        lastFrameTime = std::chrono::high_resolution_clock::now();

        // --- Main Graphics Pipeline Execution --- //

        // 1. Capture the next frame.
        HRESULT captureResult = m_capturePipeline->CaptureNextFrame();
        if (FAILED(captureResult)) {
            // If capture fails (e.g., window closed, mode switched), we can pause or log.
            // For now, just continue the loop.
            continue;
        }

        // Retrieve the captured frame textures.
        ID3D11Texture2D* currentFrame = m_capturePipeline->GetCurrentFrame();
        ID3D11Texture2D* previousFrame = m_capturePipeline->GetPreviousFrame();

        if (!currentFrame) {
            continue;
        }

        // Retrieve the current settings for this frame.
        const RXNSettings& settings = m_configManager->GetSettings();

        // 2. Execute Super Resolution (RXSR 1.0) - Placeholder
        if (settings.enableSR) {
            // Pass 'currentFrame' to the RXNSuperResolution module.
            // This module will apply its compute shader for upscaling and sharpening.
            // (Code for this step will be implemented in the next phase)
        }

        // 3. Execute Frame Generation (RXFG 1.0) - Placeholder
        if (settings.enableFG) {
            // Pass 'currentFrame' and 'previousFrame' to the RXNFrameGeneration module.
            // This module will run its pixel-displacement estimation pass.
            // (Code for this step will be implemented in the next phase)
        }

        // 4. Present the final frame.
        // In our architecture, the final processed frame will be passed to the UI
        // manager's renderer (e.g., rxn_ui_renderer_dcomp) for display.
        // For now, this step is implicit.
    }
}
