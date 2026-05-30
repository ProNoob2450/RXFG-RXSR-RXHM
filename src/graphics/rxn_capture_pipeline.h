#pragma once

#include <d3d11.h>
#include <windows.h> // For HWND
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

// Provides access to the underlying DXGI interface of a WinRT graphics object.
struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
IDirect3DDxgiInterfaceAccess : ::IUnknown {
    virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};

class RXNCapturePipeline {
public:
    RXNCapturePipeline();
    ~RXNCapturePipeline();

    /**
     * @brief Initializes D3D11 and the WinRT capture session for a specific window.
     * @param target_hwnd The handle to the window to be captured.
     * @return HRESULT indicating success or failure.
     */
    HRESULT Initialize(HWND target_hwnd);

    /**
     * @brief Releases all D3D11 and WinRT resources.
     */
    void Shutdown();

    /**
     * @brief Attempts to capture the latest frame from the session.
     * @param ppFrame Pointer to receive the captured D3D11 texture. The caller is responsible for releasing it.
     * @return HRESULT indicating success, or E_FAIL if no new frame is available.
     */
    HRESULT CaptureFrame(ID3D11Texture2D** ppFrame);

private:
    // --- D3D11 Core Components ---
    ID3D11Device* m_d3d11Device;
    ID3D11DeviceContext* m_d3d11DeviceContext;
    IDXGISwapChain* m_swapChain; // Note: Not used in this implementation, but kept for sync.

    // --- WinRT Graphics Capture Components ---
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_captureItem{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };
};