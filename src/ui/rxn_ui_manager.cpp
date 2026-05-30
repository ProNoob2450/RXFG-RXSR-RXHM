#include "rxn_ui_manager.h"
#include "../main/rxn_config.h" // For RXNConfig and its settings

// A global pointer to the manager instance to be accessible from the static WndProc.
// This is a common pattern in Win32 C++ applications.
static RXNUIManager* g_uiManagerInstance = nullptr;

RXNUIManager::RXNUIManager() : m_stopFlag(false), m_hInstance(nullptr), m_configManager(nullptr) {
    g_uiManagerInstance = this;
}

RXNUIManager::~RXNUIManager() {
    Stop();
    g_uiManagerInstance = nullptr;
}

bool RXNUIManager::Initialize(HINSTANCE hInstance, RXNConfig* config_manager) {
    m_hInstance = hInstance;
    m_configManager = config_manager;
    return (m_hInstance != nullptr && m_configManager != nullptr);
}

void RXNUIManager::Start() {
    if (m_uiThread.joinable()) {
        return; // Already running
    }
    m_stopFlag.store(false);
    m_uiThread = std::thread(&RXNUIManager::UIThreadFunction, this);
}

void RXNUIManager::Stop() {
    m_stopFlag.store(true);
    // Post a quit message to unblock the message loop if it's waiting.
    if (m_hwnd) {
        PostMessage(m_hwnd, WM_QUIT, 0, 0);
    }
    if (m_uiThread.joinable()) {
        m_uiThread.join();
    }
}

HWND RXNUIManager::GetWindowHandle() const {
    return m_hwnd;
}

void RXNUIManager::UIThreadFunction() {
    // 1. Register the window class.
    const wchar_t CLASS_NAME[] = L"RXN Dashboard Class";

    WNDCLASSEXW wc = { };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = RXNUIManager::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        // Handle error
        return;
    }

    // 2. Create the window.
    m_hwnd = CreateWindowExW(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"RXN Dashboard",              // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 200,

        NULL,       // Parent window    
        NULL,       // Menu
        m_hInstance,  // Instance handle
        this        // Additional application data
    );

    if (m_hwnd == NULL) {
        return;
    }

    // Retrieve the initial settings to configure the UI.
    const RXNSettings& settings = m_configManager->GetSettings();

    // 3. Create the UI controls.
    m_hwndToggleSR = CreateWindowW(L"BUTTON", L"Enable Super Resolution (SR)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 10, 250, 25, m_hwnd, (HMENU)IDC_TOGGLE_SR, m_hInstance, NULL);
    CheckDlgButton(m_hwnd, IDC_TOGGLE_SR, settings.enableSR ? BST_CHECKED : BST_UNCHECKED);

    m_hwndToggleFG = CreateWindowW(L"BUTTON", L"Enable Frame Generation (FG)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 50, 250, 25, m_hwnd, (HMENU)IDC_TOGGLE_FG, m_hInstance, NULL);
    CheckDlgButton(m_hwnd, IDC_TOGGLE_FG, settings.enableFG ? BST_CHECKED : BST_UNCHECKED);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    // 4. Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (m_stopFlag.load()) {
            break; 
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(m_hwnd);
    UnregisterClassW(CLASS_NAME, m_hInstance);
}

LRESULT CALLBACK RXNUIManager::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDC_TOGGLE_SR:
        {
            if (g_uiManagerInstance && g_uiManagerInstance->m_configManager) {
                bool isChecked = (IsDlgButtonChecked(hwnd, IDC_TOGGLE_SR) == BST_CHECKED);
                RXNSettings currentSettings = g_uiManagerInstance->m_configManager->GetSettings();
                currentSettings.enableSR = isChecked;
                g_uiManagerInstance->m_configManager->SaveSettings(currentSettings);
            }
        }
        break;
        case IDC_TOGGLE_FG:
        {
            if (g_uiManagerInstance && g_uiManagerInstance->m_configManager) {
                bool isChecked = (IsDlgButtonChecked(hwnd, IDC_TOGGLE_FG) == BST_CHECKED);
                RXNSettings currentSettings = g_uiManagerInstance->m_configManager->GetSettings();
                currentSettings.enableFG = isChecked;
                g_uiManagerInstance->m_configManager->SaveSettings(currentSettings);
            }
        }
        break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
