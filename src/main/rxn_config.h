#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declaration to avoid including the full profile header here.
struct RXNProfile;

enum class SystemBackdropType {
    None,    // No backdrop
    Mica,    // Recommended for main application windows
    Acrylic, // Recommended for transient UI (popups, dialogs)
    MicaAlt  // A variant of Mica
};

struct RXNSettings {
    int targetFramerate = 60;
    bool enableSuperResolution = false;
    float superResolutionUpscaleFactor = 1.5f;
    SystemBackdropType backdropPreference = SystemBackdropType::Mica;
};

class RXNConfig {
public:
    RXNConfig();
    ~RXNConfig();

    // Initializes and loads the configuration.
    void Initialize(const std::wstring& config_path);

    // Saves the current configuration to disk.
    void Save();

    // Retrieves a mutable reference to the settings.
    RXNSettings& GetSettings();

    // Retrieves a constant reference to the settings.
    const RXNSettings& GetSettings() const;

private:
    RXNSettings m_settings;
    std::wstring m_configFilePath;
    // Using a pointer to a vector of profiles to avoid issues with incomplete types
    // and to better manage memory/lifetime.
    std::unique_ptr<std::vector<RXNProfile>> m_profiles;
};
