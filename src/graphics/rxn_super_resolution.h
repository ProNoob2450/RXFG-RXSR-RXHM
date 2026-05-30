#pragma once

#include <d3d11.h>

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

    // Initializes the SR system, creates the compute shader and constant buffer.
    // Must be called once before Execute can be used.
    bool Initialize(ID3D11Device* device);

    // Tears down all DirectX resources.
    void Shutdown();

    // Executes the spatial upscaling and sharpening compute shader.
    // Takes an input frame and returns a texture containing the upscaled result.
    ID3D11Texture2D* Execute(
        ID3D11DeviceContext* context,
        ID3D11Texture2D* input_frame,
        const RXNSettings& settings,
        RXSR_ScalingProfile profile
    );

private:
    // Helper function to release D3D resources safely.
    template<typename T>
    void SafeRelease(T*& ptr) {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }

    // Helper to create or resize the output texture if needed.
    bool CheckAndCreateResources(ID3D11Device* device, ID3D11Texture2D* input_frame, RXSR_ScalingProfile profile);

    // --- DirectX 11 Resources ---
    ID3D11ComputeShader* m_computeShader = nullptr;
    ID3D11Buffer* m_constantBuffer = nullptr;

    // Output resources
    ID3D11Texture2D* m_upscaledTexture = nullptr;
    ID3D11UnorderedAccessView* m_upscaledTextureUAV = nullptr;

    // Keep track of the last configuration to avoid re-creating resources every frame.
    UINT m_lastInputWidth = 0;
    UINT m_lastInputHeight = 0;
    RXSR_ScalingProfile m_lastProfile = RXSR_ScalingProfile::Off;

    // Struct that maps to the cbuffer in the compute shader.
    struct CS_Constants {
        unsigned int inputWidth;
        unsigned int inputHeight;
        unsigned int outputWidth;
        unsigned int outputHeight;
        float sharpeningAmount; // 0.0 (soft) to 1.0 (sharp)
        float _pad[3];          // Padding to ensure 16-byte alignment
    };
};
