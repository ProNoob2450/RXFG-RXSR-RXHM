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
    /**
     * @brief The main loop for graphics processing.
     */
    void GraphicsLoop();

    /**
     * @brief Shuts down and releases all graphics resources.
     */
    void Shutdown();

    // -- Core Members --
    std::thread m_graphicsThread;
    std::atomic<bool> m_stopFlag;
    
    // -- Configuration & State --
    RXNConfig* m_configManager; // Non-owning pointer
    HWND m_targetHWND;
    std::atomic<int> m_targetFramerate;

    // -- Pipelines & Renderers --
    std::unique_ptr<RXNCapturePipeline> m_capturePipeline;
};
