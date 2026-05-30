#pragma once
#include <thread>
#include <memory>
#include <atomic>
#include <windows.h> // For HWND

class RXNConfig; // Forward declaration
class RXNCapturePipeline;

class RXNGraphicsEngine {
public:
    RXNGraphicsEngine();
    ~RXNGraphicsEngine();

    /**
     * @brief Initializes the graphics engine.
     * @param target_hwnd The window handle to capture.
     * @param config_manager Pointer to the configuration manager.
     * @return True if initialization was successful, false otherwise.
     */
    bool Initialize(HWND target_hwnd, RXNConfig* config_manager);

    /**
     * @brief Starts the graphics processing loop in a separate thread.
     */
    void Start();

    /**
     * @brief Stops the graphics processing loop and waits for the thread to exit.
     */
    void Stop();

    /**
     * @brief Sets the target framerate for the graphics loop.
     * @param fps The desired frames per second.
     */
    void SetTargetFramerate(int fps);

private:
    // Flag to signal the graphics thread to stop.
    std::atomic<bool> m_stopFlag;

    // Thread for the graphics processing loop.
    std::thread m_graphicsThread;

    // Pointer to the configuration manager. Not owned by this class.
    RXNConfig* m_configManager;

    // Handle to the target window for capture.
    HWND m_targetWindow;

    // The capture pipeline instance. Owned by this class.
    std::unique_ptr<RXNCapturePipeline> m_capturePipeline;

    // Target framerate for frame timing.
    int m_targetFramerate; 
};
