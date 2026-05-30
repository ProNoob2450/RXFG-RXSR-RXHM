#pragma once

#include <Windows.h>
#include <thread>
#include <atomic>

// Forward declaration
class RXNConfig;

// Control IDs
#define IDC_TOGGLE_SR 1001
#define IDC_LABEL_SR 1003

class RXNUIManager {
public:
    RXNUIManager();
    ~RXNUIManager();

    bool Initialize(HINSTANCE hInstance, RXNConfig* config_manager);
    void Start();
    void Stop();
    HWND GetWindowHandle() const;

    // Accessor for the config manager, used by the global WndProc
    RXNConfig* GetConfig();

private:
    void UIThreadFunction();

    std::thread m_uiThread;
    std::atomic<bool> m_stopFlag;

    HINSTANCE m_hInstance;
    HWND m_hWnd;
    RXNConfig* m_configManager; // Non-owning pointer
};
