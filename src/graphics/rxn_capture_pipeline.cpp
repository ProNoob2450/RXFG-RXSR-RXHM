#include "rxn_capture_pipeline.h"
#include <windows.graphics.capture.interop.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <d3d11.h>

// Required for CreateDirect3D11DeviceFromDXGIDevice
#include <windows.graphics.directx.direct3d11.interop.h>

// Non-standard but required for interop with certain SDK versions
#include <initguid.h>

// Helper to convert a COM object (like ID3D11Device) to a WinRT object
// This is necessary to bridge the gap between classic COM D3D11 and WinRT's capture APIs.
template <typename T>
auto GetDXGIInterfaceFromObject(winrt::com_ptr<IUnknown> const& object) -> T
{
    auto dxgiDevice = object.as<IDXGIDevice>();
    if (!dxgiDevice) return nullptr;

    winrt::com_ptr<::IInspectable> inspectable;
    if (FAILED(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put()))) {
        return nullptr;
    }

    return inspectable.as<T>();
}

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
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &m_d3d11Device,
        nullptr,
        &m_d3d11DeviceContext
    );
    if (FAILED(hr)) return hr;

    // --- 2. WinRT Capture API Initialization ---
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.as<IGraphicsCaptureItemInterop>();
    if (!interop) return E_NOINTERFACE;

    hr = interop->CreateForWindow(target_hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), winrt::put_abi(m_captureItem));
    if (FAILED(hr)) return hr;

    // --- 3. Create the Frame Pool & Session ---
    auto d3dDevice = GetDXGIInterfaceFromObject<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>(winrt::com_ptr<IUnknown>(m_d3d11Device));
    if (!d3dDevice) return E_FAIL;

    m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
        d3dDevice,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2, // Number of buffers
        m_captureItem.Size()
    );

    m_session = m_framePool.CreateCaptureSession(m_captureItem);
    m_session.StartCapture();

    return S_OK;
}

void RXNCapturePipeline::Shutdown() {
    if (m_session) m_session.Close();
    if (m_framePool) m_framePool.Close();

    m_session = nullptr;
    m_framePool = nullptr;
    m_captureItem = nullptr;

    if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
    if (m_d3d11DeviceContext) { m_d3d11DeviceContext->Release(); m_d3d11DeviceContext = nullptr; }
    if (m_d3d11Device) { m_d3d11Device->Release(); m_d3d11Device = nullptr; }
}

HRESULT RXNCapturePipeline::CaptureFrame(ID3D11Texture2D** ppFrame) {
    if (!ppFrame || !m_framePool) return E_INVALIDARG;

    auto frame = m_framePool.TryGetNextFrame();
    if (!frame) return E_FAIL;

    auto surface = frame.Surface();
    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    if (!access) return E_NOINTERFACE;

    ID3D11Texture2D* capturedTexture = nullptr;
    HRESULT hr = access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), reinterpret_cast<void**>(&capturedTexture));

    if (SUCCEEDED(hr)) {
        *ppFrame = capturedTexture;
    }

    return hr;
}