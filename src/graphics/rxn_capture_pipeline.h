#pragma once

#include <d3d11.h>
#include <windows.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

// Forward declarations
struct IDirect3DDxgiInterfaceAccess;

// Helper to get a WinRT object from a COM interface
template <typename T>
auto GetDXGIInterfaceFromObject(winrt::com_ptr<IUnknown> const& object) -> T;

class RXNCapturePipeline {
public:
    RXNCapturePipeline();
    ~RXNCapturePipeline();

    HRESULT Initialize(HWND target_hwnd);
    void Shutdown();
    HRESULT CaptureFrame(ID3D11Texture2D** ppFrame);

private:
    ID3D11Device* m_d3d11Device;
    ID3D11DeviceContext* m_d3d11DeviceContext;
    IDXGISwapChain* m_swapChain;

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_captureItem{nullptr};
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{nullptr};
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{nullptr};
};