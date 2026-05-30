#include "rxn_ui_renderer_dwm.h"
#include <dwmapi.h> // For DWM APIs

// Link the DWM library
#pragma comment(lib, "dwmapi.lib")

RXNUIRendererDWM::RXNUIRendererDWM() : m_isInitialized(false) {}

void RXNUIRendererDWM::Initialize() {
    // DWM is available on Windows Vista and later.
    // No explicit initialization is required for basic features.
    m_isInitialized = true;
}

void RXNUIRendererDWM::ApplyBackdrop(HWND hwnd, SystemBackdropType type) {
    if (!m_isInitialized) return;

    // Use the official DWM_SYSTEMBACKDROP_TYPE from the SDK
    DWM_SYSTEMBACKDROP_TYPE dwm_backdrop_type = DWMSBT_AUTO;

    switch (type) {
        case SystemBackdropType::None:
            // DWMSBT_AUTO will disable the backdrop if already applied
            dwm_backdrop_type = DWMSBT_AUTO;
            break;
        case SystemBackdropType::Mica:
            // Corresponds to "Mica"
            dwm_backdrop_type = DWMSBT_MAINWINDOW;
            break;
        case SystemBackdropType::Acrylic:
            // Corresponds to "Acrylic"
            dwm_backdrop_type = DWMSBT_TRANSIENTWINDOW;
            break;
        case SystemBackdropType::MicaAlt:
            // Corresponds to "Mica Alt"
            dwm_backdrop_type = DWMSBT_TABBEDWINDOW;
            break;
    }

    // DwmSetWindowAttribute is the API to control this feature.
    DwmSetWindowAttribute(hwnd, 
                          DWMWA_SYSTEMBACKDROP_TYPE, 
                          &dwm_backdrop_type, 
                          sizeof(dwm_backdrop_type));
}

void RXNUIRendererDWM::Shutdown() {
    // No explicit shutdown needed for DWM.
    m_isInitialized = false;
}
