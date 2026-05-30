#include "rxn_capture_pipeline.h"

// Link necessary libraries for DXGI, D3D11, and WGC
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowsapp.lib") // For WGC

RXNCapturePipeline::RXNCapturePipeline()
    : m_device(nullptr), m_context(nullptr), m_dxgiFactory(nullptr),
      m_wgcSession(nullptr), m_dxgiDuplication(nullptr), m_isInitialized(false),
      m_activeApi(CaptureApi::Uninitialized), m_targetWindow(nullptr) {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
}

RXNCapturePipeline::~RXNCapturePipeline() {
    Shutdown();
}

HRESULT RXNCapturePipeline::Initialize(HWND target_hwnd) {
    if (m_isInitialized) {
        return S_OK; // Already initialized
    }

    m_targetWindow = target_hwnd;
    HRESULT hr = InitD3D11();
    if (FAILED(hr)) {
        return hr;
    }

    // Attempt to use WGC first if a valid window handle is provided
    if (m_targetWindow) {
        hr = InitWgc(m_targetWindow);
        if (SUCCEEDED(hr)) {
            m_activeApi = CaptureApi::WindowsGraphicsCapture;
            m_isInitialized = true;
            return S_OK;
        }
    }

    // Fallback to DXGI Desktop Duplication
    hr = InitDxgiDuplication();
    if (SUCCEEDED(hr)) {
        m_activeApi = CaptureApi::DxgiDesktopDuplication;
        m_isInitialized = true;
        return S_OK;
    }

    // If both failed, cleanup and return error
    Shutdown();
    return E_FAIL;
}

void RXNCapturePipeline::Shutdown() {
    CleanupWgc();
    CleanupDxgi();

    for (int i = 0; i < 2; ++i) {
        SafeRelease(m_frameBuffer[i]);
    }

    SafeRelease(m_context);
    SafeRelease(m_device);
    SafeRelease(m_dxgiFactory);

    m_isInitialized = false;
    m_activeApi = CaptureApi::Uninitialized;
    winrt::clear_apartment();
}

ID3D11Texture2D* RXNCapturePipeline::GetCurrentFrame() {
    if (!m_isInitialized) return nullptr;
    return m_frameBuffer[m_currentFrameIndex];
}

ID3D11Texture2D* RXNCapturePipeline::GetPreviousFrame() {
    if (!m_isInitialized) return nullptr;
    return m_frameBuffer[1 - m_currentFrameIndex];
}

HRESULT RXNCapturePipeline::CaptureNextFrame() {
    if (!m_isInitialized) {
        return E_FAIL;
    }

    // Swap frame buffer index
    m_currentFrameIndex = 1 - m_currentFrameIndex;

    if (m_activeApi == CaptureApi::WindowsGraphicsCapture) {
        // WGC is asynchronous, frame pool handles new frames.
        // We just need to get the latest one.
        auto frame = m_wgcFramePool->TryGetNextFrame();
        if (!frame) {
            return DXGI_ERROR_WAIT_TIMEOUT; // No new frame yet
        }

        // Get texture from the frame
        auto sourceTexture = frame.as<winrt::Windows::Graphics::Capture::IDirect3D11CaptureFrame>()->Surface();
        
        // Copy to our corresponding frame buffer
        // This copy is GPU-side, no CPU involvement
        m_context->CopyResource(m_frameBuffer[m_currentFrameIndex], sourceTexture.get());
        return S_OK;
    }

    if (m_activeApi == CaptureApi::DxgiDesktopDuplication) {
        HRESULT hr;
        // Release the previous frame before acquiring the next one.
        if (m_dxgiAcquiredDesktopImage) {
             hr = m_dxgiDuplication->ReleaseFrame();
        }
        
        hr = m_dxgiDuplication->AcquireNextFrame(50, &m_dxgiFrameInfo, (IDXGIResource**)&m_dxgiAcquiredDesktopImage);
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            return hr; // Nothing new to capture
        }
        if (FAILED(hr)) {
            // If the desktop mode changed or something went wrong, re-initialize.
            CleanupDxgi();
            InitDxgiDuplication();
            return hr;
        }

        // Copy to our frame buffer
        if (m_dxgiAcquiredDesktopImage) {
            m_context->CopyResource(m_frameBuffer[m_currentFrameIndex], m_dxgiAcquiredDesktopImage);
        }
        return S_OK;
    }

    return E_FAIL;
}

// --- Private Initializer Methods ---

HRESULT RXNCapturePipeline::InitD3D11() {
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    HRESULT hr = D3D11CreateDevice(
        nullptr, // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        nullptr, // We don't need to check the feature level
        &m_context
    );

    if (FAILED(hr)) {
        return hr;
    }

    return CreateDXGIFactory2(0, __uuidof(IDXGIFactory6), (void**)&m_dxgiFactory);
}

HRESULT RXNCapturePipeline::InitWgc(HWND hwnd) {
    // Get the interop factory
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.as<IGraphicsCaptureItemInterop>();

    // Create a GraphicsCaptureItem from the HWND
    winrt::check_hresult(interop->CreateForWindow(hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(m_wgcItem)));
    if (!m_wgcItem) {
        return E_FAIL;
    }

    // Create the frame pool
    auto d3dDevice = m_device;
    m_wgcFramePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
        d3dDevice,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2, // Number of buffers
        m_wgcItem.as<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>().Size()
    );

    m_wgcSession = m_wgcFramePool.as<winrt::Windows::Graphics::Capture::IGraphicsCaptureSession>();
    m_wgcSession->StartCapture();

    m_wgcFrameSize = m_wgcItem.as<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>().Size();
    
    // Create our internal frame buffers to match capture size
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_wgcFrameSize.Width;
    desc.Height = m_wgcFrameSize.Height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    for (int i = 0; i < 2; ++i) {
        m_device->CreateTexture2D(&desc, nullptr, &m_frameBuffer[i]);
    }

    return S_OK;
}

HRESULT RXNCapturePipeline::InitDxgiDuplication() {
    if (!m_dxgiFactory) return E_FAIL;

    // Find the primary adapter and output
    IDXGIAdapter1* adapter = nullptr;
    IDXGIOutput* output = nullptr;
    HRESULT hr = m_dxgiFactory->EnumAdapters1(0, &adapter);
    if (FAILED(hr)) return hr;

    hr = adapter->EnumOutputs(m_dxgiOutputNumber, &output);
    SafeRelease(adapter);
    if (FAILED(hr)) return hr;

    auto output6 = output->QueryInterface<IDXGIOutput6>();
    SafeRelease(output);
    if (!output6) return E_FAIL;

    hr = output6->DuplicateOutput(m_device, &m_dxgiDuplication);
    SafeRelease(output6);
    if (FAILED(hr)) return hr;

    DXGI_OUTDUPL_DESC duplDesc;
    m_dxgiDuplication->GetDesc(&duplDesc);

    // Create our internal frame buffers to match capture size
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = duplDesc.ModeDesc.Width;
    desc.Height = duplDesc.ModeDesc.Height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = duplDesc.ModeDesc.Format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;

    for (int i = 0; i < 2; ++i) {
        m_device->CreateTexture2D(&desc, nullptr, &m_frameBuffer[i]);
    }

    return S_OK;
}

void RXNCapturePipeline::CleanupWgc() {
    if (m_wgcSession) {
        m_wgcSession->Close();
        m_wgcSession = nullptr;
    }
    m_wgcFramePool = nullptr;
    m_wgcItem = nullptr;
}

void RXNCapturePipeline::CleanupDxgi() {
    SafeRelease(m_dxgiAcquiredDesktopImage);
    SafeRelease(m_dxgiDuplication);
}
