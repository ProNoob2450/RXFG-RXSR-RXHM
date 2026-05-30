#pragma once

#include <Windows.h>
#include <dwmapi.h>
#include "../main/rxn_config.h" // For UIMode enum

class RXNUIRendererDWM {
public:
    RXNUIRendererDWM();
    ~RXNUIRendererDWM();

    // Initializes the renderer with the target window handle.
    bool Initialize(HWND hwnd);

    // Applies the selected backdrop effect based on the UI mode.
    // This should be called after initialization and whenever the mode changes.
    bool ApplyBackdropEffect(UIMode mode);

    // In a full implementation, this would handle DWM-specific window messages
    // to toggle effects when the window becomes inactive, for example.
    void HandleWindowMessage(UINT uMsg, WPARAM wParam);

private:
    HWND m_hwnd = nullptr;
};
