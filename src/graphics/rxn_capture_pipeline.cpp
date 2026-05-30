#include "rxn_capture_pipeline.h"
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h> // For WinRT D3D11 types
#include <windows.graphics.capture.interop.h>       // For IGraphicsCaptureItemInterop
#include <windows.graphics.directx.direct3d11.h>    // For IDirect3DDxgiInterfaceAccess

// Link necessary libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowsapp.lib")

// Helper to get the D3D11 texture from a WinRT Direct3D surface
static winrt::com_ptr<ID3D11Texture2D> GetTextureFromSurface(
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface const& surface)
{
    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    winrt::com_ptr<ID3D11Texture2D> texture;
    winrt::check_hresult(access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), texture.put_void()));
    return texture;
}

RXNCapturePipeline::RXNCapturePipeline()
    : m_device(nullptr), m_context(nullptr), m_dxgiFactory(nullptr),
      m_wgcSession(nullptr), m_dxgiDuplication(nullptr), m_isInitialized(false),
      m_activeApi(CaptureApi::Uninitialized), m_targetWindow(nullptr) {
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    } catch (winrt::hresult_error const&) {
        // Apartment already initialized, which is fine.
    }
}

RXNCapturePipeline::~RXNCapturePipeline() {
    Shutdown();
}

HRESULT RXNCapturePipeline::Initialize(HWND target_hwnd) {
    if (m_isInitialized) {
        return S_OK;
    }

    m_targetWindow = target_hwnd;
    HRESULT hr = InitD3D11();
    if (FAILED(hr)) {
        Shutdown();
        return hr;
    }

    // Try to initialize with Windows Graphics Capture first.
    if (target_hwnd != nullptr && SUCCEEDED(InitWgc(target_hwnd))) {
        m_activeApi = CaptureApi::WindowsGraphicsCapture;
    } else {
        // Fallback to DXGI Desktop Duplication
        if (SUCCEEDED(InitDxgi())) {
            m_activeApi = CaptureApi::DxgiDesktopDuplication;
        } else {
            Shutdown();
            return E_FAIL; // Both methods failed
        }
    }

    m_isInitialized = (m_activeApi != CaptureApi::Uninitialized);
    return m_isInitialized ? S_OK : E_FAIL;
}

void RXNCapturePipeline::Shutdown() {
    // Stop WGC session
    if (m_wgcSession) {
        m_wgcSession.Close();
        m_wgcSession = nullptr;
    }
    if (m_wgcFramePool) {
        m_wgcFramePool.Close();
        m_wgcFramePool = nullptr;
    }
    m_wgcItem = nullptr;

    // Release DXGI resources
    if (m_dxgiDuplication) {
        m_dxgiDuplication->Release();
        m_dxgiDuplication = nullptr;
    }

    // Release D3D11 resources
    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
    if (m_dxgiFactory) {
        m_dxgiFactory->Release();
        m_dxgiFactory = nullptr;
    }

    m_isInitialized = false;
    m_activeApi = CaptureApi::Uninitialized;
    
    // CORRECTED: Uninitialize the WinRT apartment.
    winrt::uninit_apartment();
}

HRESULT RXNCapturePipeline::CaptureFrame(ID3D11Texture2D** out_frame) {
    if (!m_isInitialized || !out_frame) {
        return E_FAIL;
    }

    if (m_activeApi == CaptureApi::WindowsGraphicsCapture) {
        // CORRECTED: Use TryGetNextFrame() which is the correct, modern non-blocking call.
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame frame = m_wgcFramePool.TryGetNextFrame();
        if (frame) {
            auto surfaceTexture = GetTextureFromSurface(frame.Surface());
            *out_frame = surfaceTexture.detach(); // Transfer ownership
            return S_OK;
        }
        return E_FAIL; // No frame available
    } 
    else if (m_activeApi == CaptureApi::DxgiDesktopDuplication) {
        DXGI_OUTDUPL_FRAME_INFO frame_info;
        IDXGIResource* desktop_resource = nullptr;
        HRESULT hr = m_dxgiDuplication->AcquireNextFrame(500, &frame_info, &desktop_resource);

        if (SUCCEEDED(hr) && desktop_resource) {
            ID3D11Texture2D* desktop_texture = nullptr;
            hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&desktop_texture));
            desktop_resource->Release();

            if (SUCCEEDED(hr)) {
                *out_frame = desktop_texture; // Transfer ownership
                 m_dxgiDuplication->ReleaseFrame();
                return S_OK;
            }
        }
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            return S_OK; // Not an error, just no update
        }
        m_dxgiDuplication->ReleaseFrame();
        return E_FAIL;
    }

    return E_FAIL;
}

HRESULT RXNCapturePipeline::InitD3D11() {
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

    return D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
        &m_device, nullptr, &m_context);
}

HRESULT RXNCapturePipeline::InitWgc(HWND hwnd) {
    if (!winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported()) {
        return E_FAIL;
    }
    
    // CORRECTED: Use try_as to safely get the interop interface
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.try_as<IGraphicsCaptureItemInterop>();
    if (!interop) {
        return E_FAIL;
    }

    // Create a GraphicsCaptureItem from the HWND
    winrt::check_hresult(interop->CreateForWindow(hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(m_wgcItem)));
    if (!m_wgcItem) {
        return E_FAIL;
    }

    // Create the frame pool. The device must be a WinRT object for CreateFreeThreaded.
    winrt::com_ptr<ID3D11Device> d3d_device_ptr(m_device);
    auto d3d_device_winrt = d3d_device_ptr.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    // CORRECTED: Use m_wgcItem.Size() directly without the failing .as<>() call
    m_wgcFramePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
        d3d_device_winrt,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2, // Number of buffers
        m_wgcItem.Size()
    );

    if (!m_wgcFramePool) {
        return E_FAIL;
    }

    // Create the capture session
    m_wgcSession = m_wgcFramePool.CreateCaptureSession(m_wgcItem);
    if (!m_wgcSession) {
        return E_FAIL;
    }

    // Start capturing
    m_wgcSession.StartCapture();
    return S_OK;
}

HRESULT RXNCapturePipeline::InitDxgi() {
    if (!m_device) return E_FAIL;

    // Get DXGI Factory
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&m_dxgiFactory);
    if (FAILED(hr)) return hr;

    // Get Adapter
    IDXGIAdapter1* adapter = nullptr;
    hr = m_dxgiFactory->EnumAdapters1(0, &adapter);
    if (FAILED(hr)) return hr;

    // Get Output
    IDXGIOutput* output = nullptr;
    hr = adapter->EnumOutputs(0, &output);
    adapter->Release();
    if (FAILED(hr)) return hr;

    // Get Output1 for Desktop Duplication
    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release();
    if (FAILED(hr)) return hr;

    // Create Desktop Duplication
    hr = output1->DuplicateOutput(m_device, &m_dxgiDuplication);
    output1->Release();
    
    return hr;
}