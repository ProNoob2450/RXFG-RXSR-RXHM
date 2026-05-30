#include "rxn_ui_manager.h"
#include "../main/rxn_config.h" // For RXNConfig
#include <stdexcept>

// Global pointer to the UI manager instance for the WndProc
static RXNUIManager* g_uiManager = nullptr;

// Forward declare the message handler
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// --- Implementation ---

RXNUIManager::RXNUIManager()
    : m_stopFlag(false),
      m_hInstance(nullptr),
      m_hWnd(nullptr),
      m_configManager(nullptr) {}

RXNUIManager::~RXNUIManager() {
    Stop();
}

RXNConfig* RXNUIManager::GetConfig() {
    return m_configManager;
}

bool RXNUIManager::Initialize(HINSTANCE hInstance, RXNConfig* config_manager) {
    m_hInstance = hInstance;
    m_configManager = config_manager;
    g_uiManager = this; // Set the global pointer

    Start();
    return m_hWnd != nullptr;
}

void RXNUIManager::Start() {
    if (m_uiThread.joinable()) return;

    m_stopFlag = false;
    m_uiThread = std::thread(&RXNUIManager::UIThreadFunction, this);
}

void RXNUIManager::Stop() {
    m_stopFlag = true;
    if (m_uiThread.joinable()) {
        if (m_hWnd) PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        m_uiThread.join();
    }
}

HWND RXNUIManager::GetWindowHandle() const {
    return m_hWnd;
}

// --- UI Update Methods ---

void RXNUIManager::SetStatusText(const std::wstring& text) {
    if (m_hWnd) {
        HWND hStatus = GetDlgItem(m_hWnd, IDC_STATUS_TEXT);
        if (hStatus) {
            SetWindowTextW(hStatus, text.c_str());
        }
    }
}

void RXNUIManager::SetFramerate(int fps) {
    if (m_hWnd) {
        HWND hFps = GetDlgItem(m_hWnd, IDC_FRAMERATE_TEXT);
        if (hFps) {
            std::wstring fpsText = std::to_wstring(fps) + L" FPS";
            SetWindowTextW(hFps, fpsText.c_str());
        }
    }
}


void RXNUIManager::UIThreadFunction() {
    // --- 1. Register window class ---
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"RXNUIWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    // --- 2. Create the window ---
    m_hWnd = CreateWindowExW(
        0, L"RXNUIWindowClass", L"RXN Control Panel",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 320,
        nullptr, nullptr, m_hInstance, nullptr
    );

    if (!m_hWnd) {
        throw std::runtime_error("Failed to create UI window.");
    }

    // --- 3. Create Fonts ---
    HFONT hTitleFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT hGroupFont = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    // --- 4. Create controls ---

    // Title
    HWND hTitle = CreateWindowW(L"static", L"RXN DASHBOARD", WS_CHILD | WS_VISIBLE | SS_CENTER,
                      10, 10, 364, 30, m_hWnd, (HMENU)IDC_TITLE, m_hInstance, nullptr);
    SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

    // Controls Group
    HWND hGroupControls = CreateWindowW(L"button", L"Controls", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                10, 50, 364, 60, m_hWnd, (HMENU)IDC_GROUP_CONTROLS, m_hInstance, nullptr);
    SendMessage(hGroupControls, WM_SETFONT, (WPARAM)hGroupFont, TRUE);

    CreateWindowW(L"button", L"Start Capture", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  20, 70, 170, 30, m_hWnd, (HMENU)IDC_BUTTON_START_CAPTURE, m_hInstance, nullptr);
    CreateWindowW(L"button", L"Stop Capture", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  194, 70, 170, 30, m_hWnd, (HMENU)IDC_BUTTON_STOP_CAPTURE, m_hInstance, nullptr);

    // Settings Group
    HWND hGroupSettings = CreateWindowW(L"button", L"Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                               10, 120, 364, 60, m_hWnd, (HMENU)IDC_GROUP_CONTROLS, m_hInstance, nullptr);
    SendMessage(hGroupSettings, WM_SETFONT, (WPARAM)hGroupFont, TRUE);

    CreateWindowW(L"static", L"Enable Super Resolution:", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  20, 145, 160, 20, m_hWnd, (HMENU)IDC_LABEL_SR, m_hInstance, nullptr);
    CreateWindowW(L"button", L"", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                  190, 145, 20, 20, m_hWnd, (HMENU)IDC_TOGGLE_SR, m_hInstance, nullptr);

    // Status Group
    HWND hGroupStatus = CreateWindowW(L"button", L"Live Status", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                             10, 190, 364, 80, m_hWnd, (HMENU)IDC_GROUP_STATUS, m_hInstance, nullptr);
    SendMessage(hGroupStatus, WM_SETFONT, (WPARAM)hGroupFont, TRUE);
    
    CreateWindowW(L"static", L"Status:", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  20, 215, 80, 20, m_hWnd, (HMENU)IDC_STATUS_LABEL, m_hInstance, nullptr);
    CreateWindowW(L"static", L"Ready", WS_CHILD | WS_VISIBLE,
                  110, 215, 250, 20, m_hWnd, (HMENU)IDC_STATUS_TEXT, m_hInstance, nullptr);

    CreateWindowW(L"static", L"Framerate:", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  20, 240, 80, 20, m_hWnd, (HMENU)IDC_FRAMERATE_LABEL, m_hInstance, nullptr);
    CreateWindowW(L"static", L"0 FPS", WS_CHILD | WS_VISIBLE,
                  110, 240, 250, 20, m_hWnd, (HMENU)IDC_FRAMERATE_TEXT, m_hInstance, nullptr);


    // --- 5. Initialize control states ---
    if (m_configManager) {
        CheckDlgButton(m_hWnd, IDC_TOGGLE_SR, m_configManager->GetSettings().enableSuperResolution ? BST_CHECKED : BST_UNCHECKED);
    }
    EnableWindow(GetDlgItem(m_hWnd, IDC_BUTTON_STOP_CAPTURE), FALSE);


    ShowWindow(m_hWnd, SW_SHOW);

    // --- 6. Message Loop ---
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0 && !m_stopFlag) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hTitleFont);
    DeleteObject(hGroupFont);
    DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
}

// --- Global Window Procedure ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            if (g_uiManager) {
                 RXNConfig* config = g_uiManager->GetConfig();
                switch (wmId) {
                    case IDC_TOGGLE_SR:
                    {
                        if (config) {
                             bool isChecked = (IsDlgButtonChecked(hWnd, IDC_TOGGLE_SR) == BST_CHECKED);
                             config->GetSettings().enableSuperResolution = isChecked;
                             config->Save();
                        }
                        break;
                    }
                    case IDC_BUTTON_START_CAPTURE:
                    {
                        g_uiManager->SetStatusText(L"Capture starting...");
                        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_START_CAPTURE), FALSE);
                        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP_CAPTURE), TRUE);
                        break;
                    }
                    case IDC_BUTTON_STOP_CAPTURE:
                    {
                        g_uiManager->SetStatusText(L"Capture stopped.");
                        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_START_CAPTURE), TRUE);
                        EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_STOP_CAPTURE), FALSE);
                        g_uiManager->SetFramerate(0);
                        break;
                    }
                }
            }
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
