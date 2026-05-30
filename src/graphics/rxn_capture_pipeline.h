#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>

// Enum to identify the active capture API
enum class CaptureApi {
    Uninitialized,
    WindowsGraphicsCapture,
    DxgiDesktopDuplication
};

class RXNCapturePipeline {
public:
    RXNCapturePipeline();
    ~RXNCapturePipeline();

    // Initializes the capture pipeline for a specific window.
    // Tries WGC first, then falls back to DXGI.
    HRESULT Initialize(HWND target_hwnd);

    // Shuts down the pipeline and releases all resources.
    void Shutdown();

    // Captures a single frame into the output texture.
    // The caller is responsible for releasing the texture.
    HRESULT CaptureFrame(ID3D11Texture2D** out_frame);

private:
    // --- Core D3D11 and DXGI Objects ---
    ID3D11Device*           m_device;
    ID3D11DeviceContext*    m_context;
    IDXGIFactory1*          m_dxgiFactory;

    // --- Windows Graphics Capture (WGC) Objects ---
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_wgcItem{nullptr};
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_wgcFramePool{nullptr};
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_wgcSession{nullptr};

    // --- DXGI Desktop Duplication Objects ---
    IDXGIOutputDuplication* m_dxgiDuplication;

    // --- State Management ---
    bool                    m_isInitialized;
    CaptureApi              m_activeApi;
    HWND                    m_targetWindow;

    // --- Initialization Helpers ---
    HRESULT InitD3D11();
    HRESULT InitWgc(HWND hwnd);
    HRESULT InitDxgi();
};