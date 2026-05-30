#include "rxn_ui_renderer_dwm.h"

// --- DWM API Definitions for Modern Backdrops ---
// These are typically defined in newer SDKs but are included here for compatibility.

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

enum DWM_SYSTEMBACKDROP_TYPE {
    DWMSBT_AUTO = 0,            // Let DWM decide.
    DWMSBT_NONE = 1,            // No backdrop.
    DWMSBT_MAINWINDOW = 2,      // Mica
    DWMSBT_TRANSIENTWINDOW = 3, // Acrylic
    DWMSBT_TABBEDWINDOW = 4     // Tabbed Mica
};

RXNUIRendererDWM::RXNUIRendererDWM() = default;

RXNUIRendererDWM::~RXNUIRendererDWM() = default; // No cleanup needed for this simple class

bool RXNUIRendererDWM::Initialize(HWND hwnd) {
    if (!IsWindow(hwnd)) {
        return false;
    }
    m_hwnd = hwnd;
    return true;
}

bool RXNUIRendererDWM::ApplyBackdropEffect(UIMode mode) {
    if (!m_hwnd) {
        return false;
    }

    // Default to turning off any special backdrop.
    DWM_SYSTEMBACKDROP_TYPE backdrop_type = DWMSBT_NONE;

    switch (mode) {
    case UIMode::Mica:
        backdrop_type = DWMSBT_MAINWINDOW;
        break;
    case UIMode::Acrylic:
        backdrop_type = DWMSBT_TRANSIENTWINDOW;
        break;
    case UIMode::Transparent: // Map Transparent to Acrylic for a similar effect
        backdrop_type = DWMSBT_TRANSIENTWINDOW;
        break;
    case UIMode::Solid: // Solid and default fall through to DWMSBT_NONE
    default:
        break;
    }

    // Apply the attribute to the window.
    HRESULT hr = DwmSetWindowAttribute(
        m_hwnd,
        DWMWA_SYSTEMBACKDROP_TYPE,
        &backdrop_type,
        sizeof(backdrop_type)
    );

    return SUCCEEDED(hr);
}

void RXNUIRendererDWM::HandleWindowMessage(UINT uMsg, WPARAM wParam) {
    // This method is a placeholder for future enhancements.
    // A complete implementation would listen for messages like WM_ACTIVATE
    // to possibly disable the effect when the window is not in focus,
    // saving system resources, as per Microsoft's guidelines.
    // For example:
    // if (uMsg == WM_ACTIVATE) {
    //     if (LOWORD(wParam) == WA_INACTIVE) {
    //         ApplyBackdropEffect(UIMode::Solid); // Turn off effect
    //     } else {
    //         // Restore the user's chosen effect
    //     }
    // }
}
