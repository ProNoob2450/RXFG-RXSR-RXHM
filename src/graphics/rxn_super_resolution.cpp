#include "rxn_super_resolution.h"
#include "../main/rxn_config.h"
#include <d3d11.h>
#include <stdexcept>

RXNSuperResolution::RXNSuperResolution()
    : m_isInitialized(false),
      m_upscaleFactor(1.0f),
      m_d3d11Device(nullptr) {}

RXNSuperResolution::~RXNSuperResolution() {
    Shutdown();
}

HRESULT RXNSuperResolution::Initialize(ID3D11Device* device, const RXNSettings& settings) {
    if (!device) {
        return E_INVALIDARG;
    }
    m_d3d11Device = device;
    m_upscaleFactor = (settings.superResolutionUpscaleFactor >= 1.0f) ? settings.superResolutionUpscaleFactor : 1.0f;
    m_isInitialized = true;
    return S_OK; // Return HRESULT to match header
}

void RXNSuperResolution::Shutdown() {
    m_d3d11Device = nullptr;
    m_isInitialized = false;
    m_upscaleFactor = 1.0f;
}

HRESULT RXNSuperResolution::ProcessFrame(ID3D11Texture2D* pInputFrame, ID3D11Texture2D* pOutputFrame) {
    if (!m_isInitialized || !pInputFrame || !pOutputFrame || !m_d3d11Device) {
        return E_UNEXPECTED;
    }

    ID3D11DeviceContext* pContext = nullptr;
    m_d3d11Device->GetImmediateContext(&pContext);
    if (pContext) {
        pContext->CopyResource(pOutputFrame, pInputFrame);
        pContext->Release();
    }

    return S_OK;
}
