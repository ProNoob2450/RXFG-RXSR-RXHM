#include "rxn_ui_renderer_dwm.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

void RXNUIRendererDWM::ApplyBackdrop(HWND hwnd, SystemBackdropType type) {
    DWM_SYSTEMBACKDROP_TYPE dwm_type = DWMSBT_AUTO;
    switch (type) {
        case SystemBackdropType::Mica: dwm_type = DWMSBT_MAINWINDOW; break;
        case SystemBackdropType::Acrylic: dwm_type = DWMSBT_TRANSIENTWINDOW; break;
        case SystemBackdropType::MicaAlt: dwm_type = DWMSBT_TABBEDWINDOW; break;
        default: dwm_type = DWMSBT_AUTO;
    }
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &dwm_type, sizeof(dwm_type));
}