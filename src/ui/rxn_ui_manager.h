#pragma once

#include <Windows.h>
#include <thread>
#include <atomic>
#include <string> // Required for std::wstring

// Forward declaration
class RXNConfig;

// Control IDs
#define IDC_TITLE 1000
#define IDC_TOGGLE_SR 1001
#define IDC_LABEL_SR 1002
#define IDC_BUTTON_START_CAPTURE 1003
#define IDC_BUTTON_STOP_CAPTURE 1004
#define IDC_STATUS_LABEL 1005
#define IDC_STATUS_TEXT 1006
#define IDC_FRAMERATE_LABEL 1007
#define IDC_FRAMERATE_TEXT 1008
#define IDC_GROUP_CONTROLS 1009
#define IDC_GROUP_STATUS 1010

class RXNUIManager {
public:
    RXNUIManager();
    ~RXNUIManager();

    bool Initialize(HINSTANCE hInstance, RXNConfig* config_manager);
    void Start();
    void Stop();
    HWND GetWindowHandle() const;

    // --- UI Update Methods ---
    // These can be called from any thread to update the dashboard UI.
    void SetStatusText(const std::wstring& text);
    void SetFramerate(int fps);

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
