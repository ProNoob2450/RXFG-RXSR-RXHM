#include "rxn_config.h"

// In a real implementation, this would use a JSON library
// (e.g., nlohmann/json, CppCore/JSON) to parse and serialize.

RXNConfig::RXNConfig()
    : m_backdropType(SystemBackdropType::Mica),
      m_superResolutionEnabled(false),
      m_frameGenerationEnabled(false),
      m_targetWindow(nullptr) {}

// Loads settings from the specified JSON file.
bool RXNConfig::Initialize(const wchar_t* config_path) {
    m_configPath = config_path;
    LoadSettings();
    return true; // Placeholder
}

// Saves the current settings to the JSON file.
void RXNConfig::SaveSettings() {
    // In a real implementation, serialize m_backdropType, etc.,
    // to the file at m_configPath.
}

// In a real implementation, this would load from m_configPath.
void RXNConfig::LoadSettings() {
    // Placeholder: Set default values.
    m_backdropType = SystemBackdropType::Mica;
    m_superResolutionEnabled = false;
    m_frameGenerationEnabled = false;
    m_targetWindow = nullptr;
}

// --- Getters and Setters ---

SystemBackdropType RXNConfig::GetSystemBackdropType() const {
    return m_backdropType;
}

void RXNConfig::SetSystemBackdropType(SystemBackdropType type) {
    m_backdropType = type;
}

bool RXNConfig::IsSuperResolutionEnabled() const {
    return m_superResolutionEnabled;
}

void RXNConfig::SetSuperResolutionEnabled(bool enabled) {
    m_superResolutionEnabled = enabled;
}

bool RXNConfig::IsFrameGenerationEnabled() const {
    return m_frameGenerationEnabled;
}

void RXNConfig::SetFrameGenerationEnabled(bool enabled) {
    m_frameGenerationEnabled = enabled;
}

HWND RXNConfig::GetTargetWindow() const {
    return m_targetWindow;
}

void RXNConfig::SetTargetWindow(HWND hwnd) {
    m_targetWindow = hwnd;
}
