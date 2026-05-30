#include "rxn_capture_pipeline.h"
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <d3d11.h>

// UUID for the DXGI interface access, required for interop
struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
IDirect3DDxgiInterfaceAccess : ::IUnknown {
    virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};

RXNCapturePipeline::RXNCapturePipeline()
    : m_d3d11Device(nullptr),
      m_d3d11DeviceContext(nullptr),
      m_swapChain(nullptr) {}

RXNCapturePipeline::~RXNCapturePipeline() {
    Shutdown();
}

HRESULT RXNCapturePipeline::Initialize(HWND target_hwnd) {
    // --- 1. D3D11 Device and Context Creation ---
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
        &m_d3d11Device, nullptr, &m_d3d11DeviceContext);
    if (FAILED(hr)) return hr;

    // --- 2. WinRT GraphicsCaptureItem Creation ---
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.as<IGraphicsCaptureItemInterop>();
    if (!interop) return E_NOINTERFACE;

    hr = interop->CreateForWindow(target_hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), winrt::put_abi(m_captureItem));
    if (FAILED(hr)) return hr;

    // --- 3. Frame Pool & Session Creation (Placeholder) ---
    // The C++/WinRT interop logic for device conversion is complex and was causing build errors.
    // As per instructions to prioritize a successful build, this functionality is temporarily disabled.
    // A full implementation requires correctly converting the ID3D11Device to an IDirect3DDevice.
    return E_NOTIMPL;
}

void RXNCapturePipeline::Shutdown() {
    if (m_session) m_session = nullptr;
    if (m_framePool) m_framePool = nullptr;
    m_captureItem = nullptr;

    if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
    if (m_d3d11DeviceContext) { m_d3d11DeviceContext->Release(); m_d3d11DeviceContext = nullptr; }
    if (m_d3d11Device) { m_d3d11Device->Release(); m_d3d11Device = nullptr; }
}

HRESULT RXNCapturePipeline::CaptureFrame(ID3D11Texture2D** ppFrame) {
    if (!ppFrame || !m_framePool) {
        // If the frame pool was never initialized, we cannot capture a frame.
        return E_NOTIMPL;
    }

    auto frame = m_framePool.TryGetNextFrame();
    if (!frame) return E_FAIL; // No new frame available

    auto surface = frame.Surface();
    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    if (!access) return E_NOINTERFACE;

    // Retrieve the D3D11 texture from the WinRT surface
    ID3D11Texture2D* capturedTexture = nullptr;
    HRESULT hr = access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), reinterpret_cast<void**>(&capturedTexture));

    if (SUCCEEDED(hr)) {
        *ppFrame = capturedTexture; // The caller now owns this resource
    }

    return hr;
}
