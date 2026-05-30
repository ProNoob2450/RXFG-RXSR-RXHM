#include "rxn_capture_pipeline.h"
#include <windows.graphics.capture.interop.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

// Non-standard but required for interop with certain SDK versions
#include <initguid.h>

// Constructor
RXNCapturePipeline::RXNCapturePipeline() 
    : m_d3d11Device(nullptr),
      m_d3d11DeviceContext(nullptr),
      m_swapChain(nullptr) {}

// Destructor
RXNCapturePipeline::~RXNCapturePipeline() {
    Shutdown();
}

// Initialize the pipeline
HRESULT RXNCapturePipeline::Initialize(HWND target_hwnd) {
    // --- 1. D3D11 Device and Context Creation ---
    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Adapter (nullptr for default)
        D3D_DRIVER_TYPE_HARDWARE,   // Driver type
        nullptr,                    // Software module (unused)
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, // Flags
        nullptr, 0,                 // Feature levels (default)
        D3D11_SDK_VERSION,          // SDK version
        &m_d3d11Device,             // Output device
        nullptr,                    // Output feature level
        &m_d3d11DeviceContext       // Output device context
    );
    if (FAILED(hr)) return hr;

    // --- 2. WinRT Capture API Initialization ---
    // Get the IGraphicsCaptureItemInterop factory
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.as<IGraphicsCaptureItemInterop>();
    if (!interop) return E_NOINTERFACE;

    // Get the capture item for the target window
    hr = interop->CreateForWindow(target_hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), winrt::put_abi(m_captureItem));
    if (FAILED(hr)) return hr;

    // --- 3. Create the Frame Pool & Session ---
    auto d3dDevice = GetDXGIInterfaceFromObject<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>(m_d3d11Device);
    if (!d3dDevice) return E_FAIL;

    // Create the frame pool
    m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
        d3dDevice,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2, // Number of buffers
        m_captureItem.Size()
    );

    // Create the capture session
    m_session = m_framePool.CreateCaptureSession(m_captureItem);

    // Start capturing
    m_session.StartCapture();

    return S_OK;
}

// Shutdown and release resources
void RXNCapturePipeline::Shutdown() {
    if (m_session) m_session.Close();
    if (m_framePool) m_framePool.Close();

    m_session = nullptr;
    m_framePool = nullptr;
    m_captureItem = nullptr;

    if (m_swapChain) m_swapChain->Release();
    if (m_d3d11DeviceContext) m_d3d11DeviceContext->Release();
    if (m_d3d11Device) m_d3d11Device->Release();

    m_d3d11Device = nullptr;
    m_d3d11DeviceContext = nullptr;
    m_swapChain = nullptr;
}

// Capture a single frame
HRESULT RXNCapturePipeline::CaptureFrame(ID3D11Texture2D** ppFrame) {
    if (!ppFrame || !m_framePool) return E_INVALIDARG;

    auto frame = m_framePool.TryGetNextFrame();
    if (!frame) return E_FAIL; // No new frame available yet

    // Get the D3D11 texture from the frame
    auto surface = frame.Surface();
    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    if (!access) return E_NOINTERFACE;

    ID3D11Texture2D* capturedTexture = nullptr;
    HRESULT hr = access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), reinterpret_cast<void**>(&capturedTexture));

    if (SUCCEEDED(hr)) {
        *ppFrame = capturedTexture; // Transfer ownership
    }

    return hr;
}
