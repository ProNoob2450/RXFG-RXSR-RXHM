#include "rxn_graphics_engine.h"
#include "../main/rxn_config.h" // For RXNConfig
#include "rxn_capture_pipeline.h" // For RXNCapturePipeline
#include <chrono> // For std::chrono

// Constructor: Initialize all member variables
RXNGraphicsEngine::RXNGraphicsEngine()
    : m_configManager(nullptr),
      m_targetHWND(nullptr),
      m_stopFlag(false),
      m_targetFramerate(60) // Default value
{
    // m_capturePipeline is initialized via unique_ptr in Initialize()
}

// Destructor: Ensure Stop() is called to clean up the thread
RXNGraphicsEngine::~RXNGraphicsEngine() {
    Stop();
    Shutdown();
}

// Initialize graphics resources
bool RXNGraphicsEngine::Initialize(HWND target_hwnd, RXNConfig* config_manager) {
    m_targetHWND = target_hwnd;
    m_configManager = config_manager;

    if (m_configManager) {
        m_targetFramerate = m_configManager->GetSettings().targetFramerate;
    }

    // Initialize the capture pipeline
    m_capturePipeline = std::make_unique<RXNCapturePipeline>();
    if (FAILED(m_capturePipeline->Initialize(m_targetHWND))) {
        return false;
    }

    return true;
}

// Start the graphics loop thread
void RXNGraphicsEngine::Start() {
    if (m_graphicsThread.joinable()) {
        return; // Already started
    }
    m_stopFlag = false;
    m_graphicsThread = std::thread(&RXNGraphicsEngine::GraphicsLoop, this);
}

// Stop the graphics loop thread
void RXNGraphicsEngine::Stop() {
    m_stopFlag = true;
    if (m_graphicsThread.joinable()) {
        m_graphicsThread.join();
    }
}

// Set the target framerate
void RXNGraphicsEngine::SetTargetFramerate(int fps) {
    m_targetFramerate = fps;
}

// Main graphics loop
void RXNGraphicsEngine::GraphicsLoop() {
    while (!m_stopFlag) {
        // --- Main Loop Logic ---
        // 1. Capture a frame
        // 2. Process the frame (e.g., apply effects, run analysis)
        // 3. Render the frame (or present it)

        if (m_capturePipeline) {
            ID3D11Texture2D* frame = nullptr;
            if (SUCCEEDED(m_capturePipeline->CaptureFrame(&frame))) {
                if (frame) {
                    // In a real app: process the texture
                    frame->Release();
                }
            }
        }

        // --- Framerate Control ---
        int fps = m_targetFramerate.load();
        if (fps > 0) {
            auto frame_duration = std::chrono::milliseconds(1000) / fps;
            std::this_thread::sleep_for(frame_duration);
        }
    }
}

// Shutdown and release resources
void RXNGraphicsEngine::Shutdown() {
    if (m_capturePipeline) {
        m_capturePipeline->Shutdown();
        m_capturePipeline.reset(); // Release the unique_ptr
    }
    // Any other cleanup goes here
}
