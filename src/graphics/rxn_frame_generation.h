#pragma once

#include <d3d11.h>

class RXNFrameGeneration {
public:
    RXNFrameGeneration();
    ~RXNFrameGeneration();

    /**
     * @brief Initializes the frame generation module.
     * @param pDevice A pointer to the D3D11 device.
     * @return True if initialization is successful, false otherwise.
     */
    bool Initialize(ID3D11Device* pDevice);

    /**
     * @brief Shuts down the module and releases all resources.
     */
    void Shutdown();

    /**
     * @brief Generates a new frame based on the input frame.
     * @param pInputFrame The source texture.
     * @param pOutputFrame The destination texture for the generated frame.
     * @return HRESULT indicating success or failure.
     */
    HRESULT GenerateFrame(ID3D11Texture2D* pInputFrame, ID3D11Texture2D* pOutputFrame);

private:
    ID3D11Device* m_d3d11Device;
    bool m_isInitialized;
};
