#pragma once

#include <string>
#include <vector>
#include <windows.h> // For HWND

// Defines the available DWM backdrop materials for the UI.
enum class SystemBackdropType {
    None,
    Mica,
    Acrylic,
    MicaAlt
};

class RXNConfig {
public:
    RXNConfig();

    // Loads settings from the specified JSON file.
    bool Initialize(const wchar_t* config_path);

    // Saves the current settings to the JSON file.
    void SaveSettings();

    // --- Getters and Setters for Configuration Properties ---

    SystemBackdropType GetSystemBackdropType() const;
    void SetSystemBackdropType(SystemBackdropType type);

    bool IsSuperResolutionEnabled() const;
    void SetSuperResolutionEnabled(bool enabled);

    bool IsFrameGenerationEnabled() const;
    void SetFrameGenerationEnabled(bool enabled);

    HWND GetTargetWindow() const;
    void SetTargetWindow(HWND hwnd);

private:
    // Path to the configuration file.
    std::wstring m_configPath;

    // --- Configuration Properties ---

    // The type of DWM backdrop to apply (e.g., Mica, Acrylic).
    SystemBackdropType m_backdropType;

    // Flag indicating whether super resolution is enabled.
    bool m_superResolutionEnabled;

    // Flag indicating whether frame generation is enabled.
    bool m_frameGenerationEnabled;

    // The target window for the capture pipeline.
    HWND m_targetWindow;

    // Helper method to load settings from the member path.
    void LoadSettings();
};