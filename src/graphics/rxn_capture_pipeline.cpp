#include "rxn_capture_pipeline.h"
#include <windows.graphics.capture.interop.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <d3d11_4.h>

RXNCapturePipeline::RXNCapturePipeline() : m_d3d11Device(nullptr), m_d3d11DeviceContext(nullptr), m_swapChain(nullptr) {}
RXNCapturePipeline::~RXNCapturePipeline() { Shutdown(); }

HRESULT RXNCapturePipeline::Initialize(HWND target_hwnd) {
    // 1. D3D11 Setup
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, 1, D3D11_SDK_VERSION, &m_d3d11Device, nullptr, &m_d3d11DeviceContext);
    if (FAILED(hr)) return hr;

    // 2. WGC Setup
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
    hr = factory->CreateForWindow(target_hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), winrt::put_abi(m_captureItem));
    if (FAILED(hr)) return hr;

    auto dxgiDevice = m_d3d11Device.as<IDXGIDevice>();
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice winrtDevice;
    hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), reinterpret_cast<IInspectable**>(winrt::put_abi(winrtDevice)));
    if (FAILED(hr)) return hr;

    m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(winrtDevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_captureItem.Size());
    m_session = m_framePool.CreateCaptureSession(m_captureItem);
    m_session.StartCapture();

    return S_OK;
}

HRESULT RXNCapturePipeline::CaptureFrame(ID3D11Texture2D** ppFrame) {
    if (!ppFrame || !m_framePool) return E_INVALIDARG;
    auto frame = m_framePool.TryGetNextFrame();
    if (!frame) return E_FAIL;
    auto access = frame.Surface().as<IDirect3DDxgiInterfaceAccess>();
    return access->GetInterface(IID_PPV_ARGS(ppFrame));
}

void RXNCapturePipeline::Shutdown() {
    if (m_session) m_session.Close();
    if (m_framePool) m_framePool.Close();
    m_session = nullptr;
    m_framePool = nullptr;
    m_captureItem = nullptr;
    if (m_swapChain) m_swapChain->Release();
    if (m_d3d11DeviceContext) m_d3d11DeviceContext->Release();
    if (m_d3d11Device) m_d3d11Device->Release();
}