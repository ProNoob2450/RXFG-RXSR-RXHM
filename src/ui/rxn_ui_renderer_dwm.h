#pragma once
#include <windows.h>
#include "../main/rxn_config.h" // For SystemBackdropType

class RXNUIRendererDWM {
public:
    RXNUIRendererDWM();

    // Initializes the DWM renderer.
    void Initialize();

    // Applies a system backdrop material to the specified window.
    void ApplyBackdrop(HWND hwnd, SystemBackdropType type);

    // Shuts down the DWM renderer.
    void Shutdown();

private:
    bool m_isInitialized;
};
