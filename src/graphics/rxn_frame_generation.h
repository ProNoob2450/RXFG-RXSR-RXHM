#pragma once

#include <d3d11.h>
#include <atomic>

class RXNFrameGeneration {
public:
    RXNFrameGeneration();
    ~RXNFrameGeneration();

    // Initializes the FG system, creates shaders and D3D11 resources.
    // Must be called once before Execute can be used.
    bool Initialize(ID3D11Device* device);

    // Releases all D3D11 resources.
    void Shutdown();

    // Generates an interpolated frame between the previous and current frame.
    // The result is stored internally. This simulates a lightweight pixel
    // movement estimation and blending pass.
    void Execute(
        ID3D11DeviceContext* context,
        ID3D11Texture2D* currentFrame,  // Represents Frame N
        ID3D11Texture2D* previousFrame // Represents Frame N-1
    );

    // Returns the latest generated frame, if one is ready.
    // Returns nullptr if no new frame has been generated since the last call.
    // This is thread-safe and ensures a single consumer gets the frame.
    ID3D11Texture2D* GetGeneratedFrame();

private:
    // Helper function to release D3D resources safely.
    template<typename T>
    void SafeRelease(T*& ptr) {
        if (ptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }

    // Creates or resizes the output texture and its view if needed.
    // This ensures a zero-allocation path during the main loop.
    bool CheckAndCreateResources(ID3D11Device* device, ID3D11Texture2D* referenceFrame);

    // --- DirectX 11 Resources ---
    ID3D11VertexShader*     m_fullscreenVS = nullptr;
    ID3D11PixelShader*      m_blendPS = nullptr;
    ID3D11SamplerState*     m_samplerState = nullptr;

    // Output resources for the interpolated frame
    ID3D11Texture2D*        m_interpolatedTexture = nullptr;
    ID3D11RenderTargetView* m_interpolatedTextureRTV = nullptr;

    // --- State Management ---
    std::atomic<bool> m_isFrameReady;
    UINT m_lastWidth = 0;
    UINT m_lastHeight = 0;
};
