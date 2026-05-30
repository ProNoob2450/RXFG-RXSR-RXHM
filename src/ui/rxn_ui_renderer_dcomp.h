#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dcomp.h>

class RXNUIRendererDComp {
public:
    RXNUIRendererDComp();
    ~RXNUIRendererDComp();

    // Initializes the D3D11 device and DirectComposition resources.
    bool Initialize(HWND hwnd);

    // Releases all COM objects and resources.
    void Shutdown();

    // Placeholder for the main rendering call. This is where the scene
    // would be composed and effects like blur would be applied to visuals.
    bool Render();

    // Provides access to the D3D11 device for other components.
    ID3D11Device* GetD3D11Device() const;

private:
    // Helper to release COM objects safely.
    template<typename T>
    void SafeRelease(T*& ptr) {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }

    // --- Core D3D11 & DComposition Objects ---
    HWND m_hwnd = nullptr;

    ID3D11Device*           m_d3d11Device = nullptr;
    ID3D11DeviceContext*    m_d3d11Context = nullptr;
    IDXGISwapChain*         m_dxgiSwapChain = nullptr;

    IDCompositionDevice*    m_dcompDevice = nullptr;
    IDCompositionTarget*    m_dcompTarget = nullptr;
    IDCompositionVisual*    m_rootVisual = nullptr;
};
