#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <inspectable.h> // Required for IGraphicsCaptureItemInterop
#include <windows.graphics.capture.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>

// Forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain1;
struct ID3D11Texture2D;

enum class CaptureApi {
    Uninitialized,
    WindowsGraphicsCapture,
    DxgiDesktopDuplication
};

class RXNCapturePipeline {
public:
    RXNCapturePipeline();
    ~RXNCapturePipeline();

    // Initializes the D3D11 device and attempts to start WGC for a specific window.
    // Falls back to DXGI Desktop Duplication if WGC fails or HWND is null.
    HRESULT Initialize(HWND target_hwnd);

    // Tears down all resources.
    void Shutdown();

    // Retrieves the most recent frame from the capture source.
    // Returns a pointer to the current frame texture (Frame N).
    ID3D11Texture2D* GetCurrentFrame();

    // Retrieves the previously captured frame (Frame N-1).
    ID3D11Texture2D* GetPreviousFrame();

    // Performs the capture operation for one frame.
    HRESULT CaptureNextFrame();

    // Public getters
    ID3D11Device* GetDevice() const { return m_device; }
    ID3D11DeviceContext* GetContext() const { return m_context; }
    CaptureApi GetActiveApi() const { return m_activeApi; }

private:
    // --- Helper Methods ---
    HRESULT InitD3D11();
    HRESULT InitWgc(HWND hwnd);
    HRESULT InitDxgiDuplication();
    void CleanupDxgi();
    void CleanupWgc();

    template<typename T>
    void SafeRelease(T*& ptr) {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }

    // --- Core D3D11 & DXGI Components ---
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGIFactory6* m_dxgiFactory = nullptr;
    
    // --- Frame Buffers ---
    // Double buffer for temporal processing (Frame N and Frame N-1)
    ID3D11Texture2D* m_frameBuffer[2] = { nullptr, nullptr };
    int m_currentFrameIndex = 0;

    // --- Windows Graphics Capture (WGC) Components ---
    winrt::com_ptr<IInspectable> m_wgcItem;
    winrt::com_ptr<winrt::Windows::Graphics::Capture::IDirect3D11CaptureFramePool> m_wgcFramePool;
    winrt::com_ptr<winrt::Windows::Graphics::Capture::IGraphicsCaptureSession> m_wgcSession;
    winrt::Windows::Graphics::SizeInt32 m_wgcFrameSize{};

    // --- DXGI Desktop Duplication Components ---
    IDXGIOutputDuplication* m_dxgiDuplication = nullptr;
    DXGI_OUTDUPL_FRAME_INFO m_dxgiFrameInfo{};
    ID3D11Texture2D* m_dxgiAcquiredDesktopImage = nullptr;
    UINT m_dxgiOutputNumber = 0; // The monitor we are duplicating

    // --- State ---
    CaptureApi m_activeApi = CaptureApi::Uninitialized;
    bool m_isInitialized = false;
    HWND m_targetWindow = nullptr;
};
