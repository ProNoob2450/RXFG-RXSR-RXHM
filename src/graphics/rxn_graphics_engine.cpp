#include "rxn_graphics_engine.h"
#include "../main/rxn_config.h"       // To access settings
#include "rxn_capture_pipeline.h" // To access the capture API

RXNGraphicsEngine::RXNGraphicsEngine()
    : m_stopFlag(false), m_configManager(nullptr), m_targetWindow(nullptr), m_targetFramerate(60) {
    // Default to 60 FPS until set otherwise.
}

RXNGraphicsEngine::~RXNGraphicsEngine() {
    Stop();
    // Ensure capture pipeline is cleaned up.
    // std::unique_ptr will handle deletion.
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
    if (FAILED(hr)) {
        // Log error or handle failure appropriately
        return false;
    }

    // Initialization successful.
    return true;
}

void RXNGraphicsEngine::Start() {
    if (m_graphicsThread.joinable()) {
        return; // Thread is already running.
    }
    m_stopFlag = false;
    // Create and launch the graphics thread.
    m_graphicsThread = std::thread(&RXNGraphicsEngine::GraphicsLoop, this);
}

void RXNGraphicsEngine::Stop() {
    if (m_graphicsThread.joinable()) {
        m_stopFlag = true; // Signal the thread to stop.
        m_graphicsThread.join(); // Wait for the thread to finish.
    }
}

void RXNGraphicsEngine::SetTargetFramerate(int fps) {
    // Clamp FPS to a reasonable range.
    if (fps < 1) fps = 1;
    if (fps > 1000) fps = 1000; // Arbitrary high limit
    m_targetFramerate = fps;
}

void RXNGraphicsEngine::GraphicsLoop() {
    // Calculate the delay needed to achieve the target framerate.
    // Time in milliseconds.
    const std::chrono::milliseconds frame_duration(1000 / m_targetFramerate);
    auto last_frame_time = std::chrono::high_resolution_clock::now();

    while (!m_stopFlag) {
        auto start_time = std::chrono::high_resolution_clock::now();

        // 1. Capture Frame
        ID3D11Texture2D* captured_frame = nullptr;
        if (m_capturePipeline) {
             // In a real scenario, handle potential errors from CaptureFrame
            m_capturePipeline->CaptureFrame(&captured_frame);
        }

        // 2. Process Frame (e.g., apply effects, generate frames)
        ID3D11Texture2D* processed_frame = nullptr;
        if (captured_frame) {
            // Placeholder for frame processing/generation logic
            // For now, we'll just use the captured frame directly.
            processed_frame = captured_frame; 
            // In a real implementation, you might call RXNFrameGeneration here.
        }

        // 3. Render Frame (if applicable)
        if (processed_frame) {
            // Placeholder for rendering logic
            // Render processed_frame to the back buffer or final output.
        }

        // Clean up the captured frame if it was a separate resource
        if (captured_frame) captured_frame->Release();

        // Frame time limiting
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
        // Update last_frame_time for potential FPS calculation
        last_frame_time = std::chrono::high_resolution_clock::now(); 
    }
}
