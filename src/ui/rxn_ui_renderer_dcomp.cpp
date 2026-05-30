#include "rxn_ui_renderer_dcomp.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")

RXNUIRendererDComp::RXNUIRendererDComp() = default;

RXNUIRendererDComp::~RXNUIRendererDComp() {
    Shutdown();
}

bool RXNUIRendererDComp::Initialize(HWND hwnd) {
    if (!IsWindow(hwnd)) {
        return false;
    }
    m_hwnd = hwnd;

    HRESULT hr = S_OK;

    // 1. Create the D3D11 Device and Context
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Needed for DComp
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    hr = D3D11CreateDevice(
        nullptr,                    // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        createDeviceFlags,
        nullptr,                    // No specific feature levels.
        0,
        D3D11_SDK_VERSION,
        &m_d3d11Device,
        nullptr,                    // No need to check feature level.
        &m_d3d11Context
    );

    if (FAILED(hr)) return false;

    // 2. Create the DXGI Swap Chain
    IDXGIFactory* dxgiFactory = nullptr;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
    if (FAILED(hr)) return false;

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    DXGI_SWAP_CHAIN_DESC scd = { };
    scd.BufferCount = 2; // Double-buffered
    scd.BufferDesc.Width = rc.right - rc.left;
    scd.BufferDesc.Height = rc.bottom - rc.top;
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = m_hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // Modern swap effect

    hr = dxgiFactory->CreateSwapChain(m_d3d11Device, &scd, &m_dxgiSwapChain);
    SafeRelease(dxgiFactory);
    if (FAILED(hr)) return false;

    // 3. Create the DirectComposition Device
    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_d3d11Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) return false;

    hr = DCompositionCreateDevice(dxgiDevice, __uuidof(IDCompositionDevice), (void**)&m_dcompDevice);
    SafeRelease(dxgiDevice);
    if (FAILED(hr)) return false;

    // 4. Create the Composition Target
    hr = m_dcompDevice->CreateTargetForHwnd(m_hwnd, TRUE, &m_dcompTarget);
    if (FAILED(hr)) return false;

    // 5. Create the Root Visual and bind it to the swap chain
    hr = m_dcompDevice->CreateVisual(&m_rootVisual);
    if (FAILED(hr)) return false;

    hr = m_rootVisual->SetContent(m_dxgiSwapChain);
    if (FAILED(hr)) return false;

    // 6. Set the root visual on the target and commit
    hr = m_dcompTarget->SetRoot(m_rootVisual);
    if (FAILED(hr)) return false;

    hr = m_dcompDevice->Commit();
    return SUCCEEDED(hr);
}

void RXNUIRendererDComp::Shutdown() {
    SafeRelease(m_rootVisual);
    SafeRelease(m_dcompTarget);
    SafeRelease(m_dcompDevice);
    SafeRelease(m_dxgiSwapChain);
    SafeRelease(m_d3d11Context);
    SafeRelease(m_d3d11Device);
}

bool RXNUIRendererDComp::Render() {
    if (!m_dcompDevice || !m_dxgiSwapChain) {
        return false;
    }

    // This is where the rendering commands for the UI would go.
    // For example, clearing the back buffer:
    // ID3D11Texture2D* pBackBuffer = nullptr;
    // m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    // ID3D11RenderTargetView* pRenderTargetView = nullptr;
    // m_d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
    // float clearColor[4] = { 0.0f, 0.2f, 0.4f, 0.0f }; // Transparent blue
    // m_d3d11Context->ClearRenderTargetView(pRenderTargetView, clearColor);
    // pRenderTargetView->Release();
    // pBackBuffer->Release();

    // Present the frame.
    HRESULT hr = m_dxgiSwapChain->Present(1, 0);

    // Commit the composition device to apply all changes.
    hr = m_dcompDevice->Commit();

    return SUCCEEDED(hr);
}

ID3D11Device* RXNUIRendererDComp::GetD3D11Device() const {
    return m_d3d11Device;
}
