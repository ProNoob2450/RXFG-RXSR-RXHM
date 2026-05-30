#include "rxn_capture_pipeline.h"
#include <stdexcept> // For std::runtime_error

// WinRT Interop Headers
#include <windows.graphics.capture.interop.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

// This is required to get the DXGI interface from a D3D11 device.
#include <d3d11_4.h>

RXNCapturePipeline::RXNCapturePipeline()
    : m_device(nullptr),
      m_context(nullptr),
      m_dxgiFactory(nullptr),
      m_dxgiDuplication(nullptr),
      m_isInitialized(false),
      m_activeApi(CaptureApi::Uninitialized),
      m_targetWindow(nullptr) {}

RXNCapturePipeline::~RXNCapturePipeline() {
    Shutdown();
}

HRESULT RXNCapturePipeline::Initialize(HWND target_hwnd) {
    if (m_isInitialized) return S_OK;

    m_targetWindow = target_hwnd;

    HRESULT hr = InitD3D11();
    if (FAILED(hr)) return hr;

    // Try to initialize with Windows Graphics Capture first.
    hr = InitWgc(target_hwnd);
    if (SUCCEEDED(hr)) {
        m_activeApi = CaptureApi::WindowsGraphicsCapture;
        m_isInitialized = true;
        return S_OK;
    }

    // Fallback to DXGI Desktop Duplication if WGC fails.
    hr = InitDxgi();
    if (SUCCEEDED(hr)) {
        m_activeApi = CaptureApi::DxgiDesktopDuplication;
        m_isInitialized = true;
        return S_OK;
    }

    Shutdown(); // Clean up partial initializations
    return E_FAIL;
}

void RXNCapturePipeline::Shutdown() {
    // WGC Cleanup
    if (m_wgcSession) {
        m_wgcSession.Close();
        m_wgcSession = nullptr;
    }
    if (m_wgcFramePool) {
        m_wgcFramePool.Close();
        m_wgcFramePool = nullptr;
    }
    m_wgcItem = nullptr;

    // DXGI Cleanup
    if (m_dxgiDuplication) {
        m_dxgiDuplication->Release();
        m_dxgiDuplication = nullptr;
    }

    // D3D11 Cleanup
    if (m_dxgiFactory) {
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
    }
    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }

    m_isInitialized = false;
    m_activeApi = CaptureApi::Uninitialized;

    // Correctly uninitialize the WinRT apartment.
    winrt::uninit_apartment();
}

HRESULT RXNCapturePipeline::CaptureFrame(ID3D11Texture2D** out_frame) {
    if (!m_isInitialized || !out_frame) return E_POINTER;

    if (m_activeApi == CaptureApi::WindowsGraphicsCapture) {
        auto frame = m_wgcFramePool.TryGetNextFrame();
        if (!frame) return E_FAIL; // No new frame available

        // Get the texture from the frame.
        auto surface = frame.Surface();
        auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
        return access->GetInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(out_frame));
    }
    // DXGI capture logic would go here...
    
    return E_NOTIMPL;
}

// --- Initialization Helpers ---

HRESULT RXNCapturePipeline::InitD3D11() {
    // Create a D3D11 device.
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    
    return D3D11CreateDevice(
        nullptr, // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, // No software rasterizer
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        nullptr, // pFeatureLevel
        &m_context);
}

HRESULT RXNCapturePipeline::InitWgc(HWND hwnd) {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
    
    HRESULT hr = factory->CreateForWindow(hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), winrt::put_abi(m_wgcItem));
    if (FAILED(hr)) return hr;

    // Modern way to get the WinRT IDirect3DDevice
    winrt::com_ptr<IDXGIDevice> dxgiDevice = m_device.as<IDXGIDevice>();
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice winrtDevice;
    const auto iid = winrt::guid_of<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
    hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), reinterpret_cast<IInspectable**>(winrt::put_abi(winrtDevice)));
    if (FAILED(hr)) return hr;
    
    m_wgcFramePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
        winrtDevice,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2, // Number of buffers
        m_wgcItem.Size()
    );

    m_wgcSession = m_wgcFramePool.CreateCaptureSession(m_wgcItem);
    m_wgcSession.StartCapture();

    return S_OK;
}

HRESULT RXNCapturePipeline::InitDxgi() {
    // DXGI implementation is a fallback and is not fully implemented here.
    return E_NOTIMPL;
}
