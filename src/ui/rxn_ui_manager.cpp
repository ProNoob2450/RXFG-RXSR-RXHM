#include "rxn_ui_manager.h"
#include "../main/rxn_config.h"
#include <stdexcept>

static RXNUIManager* g_uiManager = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
    g_uiManager = this;
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

void RXNUIManager::UIThreadFunction() {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"RXNUIWindowClass";
    RegisterClassW(&wc);

    m_hWnd = CreateWindowExW(
        0, L"RXNUIWindowClass", L"RXN Control Panel",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 150,
        nullptr, nullptr, m_hInstance, nullptr
    );

    if (!m_hWnd) {
        throw std::runtime_error("Failed to create UI window.");
    }

    CreateWindowW(L"static", L"Super Resolution:", WS_CHILD | WS_VISIBLE,
                  10, 20, 150, 20, m_hWnd, (HMENU)IDC_LABEL_SR, m_hInstance, nullptr);
    HWND hwndSRToggle = CreateWindowW(L"button", L"", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                    170, 20, 20, 20, m_hWnd, (HMENU)IDC_TOGGLE_SR, m_hInstance, nullptr);

    if (m_configManager) {
        CheckDlgButton(m_hWnd, IDC_TOGGLE_SR, m_configManager->GetSettings().enableSuperResolution ? BST_CHECKED : BST_UNCHECKED);
    }

    ShowWindow(m_hWnd, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0 && !m_stopFlag) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            if (g_uiManager && g_uiManager->GetConfig()) {
                RXNConfig* config = g_uiManager->GetConfig();
                switch (wmId) {
                    case IDC_TOGGLE_SR:
                    {
                        bool isChecked = (IsDlgButtonChecked(hWnd, IDC_TOGGLE_SR) == BST_CHECKED);
                        config->GetSettings().enableSuperResolution = isChecked;
                        config->Save();
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
