#pragma once

#include <Windows.h>
#include <string>

// Defines the aesthetic mode for the user interface.
enum class UIMode {
    Performance, // Native DWM-rendered Mica/Acrylic effect. Zero-cost.
    Overdrive    // Custom DirectComposition/HLSL liquid glass effect. For high-end systems.
};

// Defines the spatial upscaling algorithm to be used by RXSR.
enum class UpscaleMode {
    None,        // No upscaling.
    Bicubic,     // Fast and effective bicubic filtering.
    Lanczos      // Higher-quality Lanczos filtering.
};

// A single structure to hold all application settings.
// This simplifies passing configuration data between modules.
struct RXNSettings {
    // [UI] Section
    UIMode      uiMode;

    // [Capture] Section
    bool        preferWGC;          // True to prefer WGC, false to default to DXGI.

    // [SuperResolution] Section
    bool        enableSR;           // Master toggle for RXSR 1.0.
    UpscaleMode upscaleMode;        // Which scaling filter to use.
    float       sharpeningAmount;   // 0.0f to 1.0f for the CAS pass.

    // [FrameGeneration] Section
    bool        enableFG;           // Master toggle for RXFG 1.0.
};

class RXNConfig {
public:
    // Constructor initializes the config manager with a path to the .ini file.
    RXNConfig(const wchar_t* config_filename = L"rxn_config.ini");

    // Reads all settings from the .ini file into the internal settings structure.
    // If the file doesn't exist, it creates one with default values.
    void LoadSettings();

    // Writes the current state of the internal settings structure to the .ini file.
    void SaveSettings();

    // Provides read-only access to the current settings.
    const RXNSettings& GetSettings() const;

    // Updates the internal settings structure. Call SaveSettings() to persist.
    void UpdateSettings(const RXNSettings& new_settings);

private:
    // Helper to convert boolean to string for .ini file.
    std::wstring BoolToString(bool value);

    // --- Member Variables ---
    std::wstring m_filename;   // The full path to "rxn_config.ini".
    RXNSettings m_settings;    // The in-memory cache of all settings.
};
