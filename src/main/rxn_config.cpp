#include "rxn_config.h"
#include <vector> // Required for std::make_unique<std::vector...>

// Minimal definition for RXNProfile for compilation. 
// In a real scenario, this would be in its own header.
struct RXNProfile {
    std::wstring name;
};

RXNConfig::RXNConfig() {
    // Initialize the profiles vector.
    m_profiles = std::make_unique<std::vector<RXNProfile>>();
}

RXNConfig::~RXNConfig() {
    // Dtor will automatically clean up m_profiles via unique_ptr.
}

void RXNConfig::Initialize(const std::wstring& config_path) {
    m_configFilePath = config_path;
    // In a real application, you would load the config from the path.
    // For now, we'll just use the default settings.
}

void RXNConfig::Save() {
    // In a real application, you would serialize m_settings to m_configFilePath.
}

RXNSettings& RXNConfig::GetSettings() {
    return m_settings;
}

const RXNSettings& RXNConfig::GetSettings() const {
    return m_settings;
}
