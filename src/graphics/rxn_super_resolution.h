#pragma once

#include <d3d11.h>
#include <atomic>

// Forward declaration from rxn_config.h to avoid including the full header.
struct RXNSettings;

// Defines the upscaling factor to be applied.
// This determines the final output resolution of the texture.
enum class RXSR_ScalingProfile {
    Off,              // 1.0x - No upscaling, only applies sharpening if enabled.
    Balanced = 150,   // 1.5x - A balance of performance and quality.
    Quality = 200     // 2.0x - Highest quality upscaling for high-end systems.
};

class RXNSuperResolution {
public:
    RXNSuperResolution();
    ~RXNSuperResolution();

    /**
     * @brief Initializes the SR system, preparing it for operation.
     * @param device The D3D11 device.
     * @param settings The current application settings.
     * @return HRESULT indicating success or failure.
     */
    HRESULT Initialize(ID3D11Device* device, const RXNSettings& settings);

    /**
     * @brief Tears down all SR-related DirectX resources.
     */
    void Shutdown();

    /**
     * @brief Processes a frame for super resolution.
     * @param pInputFrame The input texture to be upscaled.
     * @param pOutputFrame The output texture where the upscaled frame will be written.
     * @return HRESULT indicating success or failure.
     */
    HRESULT ProcessFrame(ID3D11Texture2D* pInputFrame, ID3D11Texture2D* pOutputFrame);

    // -- Member Variables --
private:
    std::atomic<bool> m_isInitialized;
    float m_upscaleFactor;
    ID3D11Device* m_d3d11Device; // Non-owning pointer

    // Placeholder for future SR implementation details
    // E.g., compute shaders, constant buffers, textures, etc.
};
