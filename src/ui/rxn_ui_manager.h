#pragma once

#include <Windows.h>
#include <thread>
#include <atomic>

// Forward declaration to avoid including the full header.
class RXNConfig;

// Control IDs for our UI elements.
#define IDC_TOGGLE_SR 1001
#define IDC_TOGGLE_FG 1002
#define IDC_LABEL_SR 1003
#define IDC_LABEL_FG 1004

class RXNUIManager {
public:
    RXNUIManager();
    ~RXNUIManager();

    // Initializes the manager with the application instance and config.
    bool Initialize(HINSTANCE hInstance, RXNConfig* config_manager);

    // Creates and runs the UI window and its message loop on a separate thread.
    void Start();

    // Signals the UI thread to shut down and cleans up resources.
    void Stop();

    // Retrieves the handle to the main UI window.
    HWND GetWindowHandle() const;

private:
    // The entry point and main loop for the dedicated UI thread.
    void UIThreadFunction();

    // The standard Windows Procedure for our main window.
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // --- Member Variables ---
    std::thread m_uiThread;
    std::atomic<bool> m_stopFlag;

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    RXNConfig* m_configManager = nullptr; // Non-owning pointer

    // Handles for UI controls
    HWND m_hwndToggleSR = nullptr;
    HWND m_hwndToggleFG = nullptr;
};
