#include "rxn_config.h"
#include <vector>

RXNConfig::RXNConfig(const wchar_t* config_filename) {
    // Determine the full path for the .ini file in the same directory as the executable.
    std::vector<wchar_t> path_buffer(MAX_PATH);
    GetModuleFileNameW(NULL, path_buffer.data(), MAX_PATH);
    wchar_t* last_slash = wcsrchr(path_buffer.data(), L'\\');
    if (last_slash) {
        *(last_slash + 1) = L'\0'; // Truncate to get the directory.
    }
    m_filename = std::wstring(path_buffer.data()) + config_filename;
}

void RXNConfig::LoadSettings() {
    // [UI] Section
    m_settings.uiMode = static_cast<UIMode>(GetPrivateProfileIntW(L"UI", L"UIMode", static_cast<int>(UIMode::Performance), m_filename.c_str()));

    // [Capture] Section
    m_settings.preferWGC = GetPrivateProfileIntW(L"Capture", L"PreferWGC", TRUE, m_filename.c_str()) == TRUE;

    // [SuperResolution] Section
    m_settings.enableSR = GetPrivateProfileIntW(L"SuperResolution", L"EnableSR", FALSE, m_filename.c_str()) == TRUE;
    m_settings.upscaleMode = static_cast<UpscaleMode>(GetPrivateProfileIntW(L"SuperResolution", L"UpscaleMode", static_cast<int>(UpscaleMode::Bicubic), m_filename.c_str()));
    
    wchar_t sharpening_buffer[16];
    GetPrivateProfileStringW(L"SuperResolution", L"SharpeningAmount", L"0.5", sharpening_buffer, 16, m_filename.c_str());
    m_settings.sharpeningAmount = _wtof(sharpening_buffer);

    // [FrameGeneration] Section
    m_settings.enableFG = GetPrivateProfileIntW(L"FrameGeneration", L"EnableFG", FALSE, m_filename.c_str()) == TRUE;
    
    // If the file didn't exist, these calls return the default values.
    // We immediately save to ensure the .ini file is created on the first run.
    if (GetFileAttributesW(m_filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
        SaveSettings();
    }
}

void RXNConfig::SaveSettings() {
    // [UI] Section
    WritePrivateProfileStringW(L"UI", L"UIMode", std::to_wstring(static_cast<int>(m_settings.uiMode)).c_str(), m_filename.c_str());

    // [Capture] Section
    WritePrivateProfileStringW(L"Capture", L"PreferWGC", BoolToString(m_settings.preferWGC).c_str(), m_filename.c_str());

    // [SuperResolution] Section
    WritePrivateProfileStringW(L"SuperResolution", L"EnableSR", BoolToString(m_settings.enableSR).c_str(), m_filename.c_str());
    WritePrivateProfileStringW(L"SuperResolution", L"UpscaleMode", std::to_wstring(static_cast<int>(m_settings.upscaleMode)).c_str(), m_filename.c_str());
    
    // Format float to string for saving
    std::wstring sharpening_str = std::to_wstring(m_settings.sharpeningAmount);
    sharpening_str.erase(sharpening_str.find_last_not_of(L'0') + 1, std::string::npos);
    sharpening_str.erase(sharpening_str.find_last_not_of(L'.') + 1, std::string::npos);
    WritePrivateProfileStringW(L"SuperResolution", L"SharpeningAmount", sharpening_str.c_str(), m_filename.c_str());

    // [FrameGeneration] Section
    WritePrivateProfileStringW(L"FrameGeneration", L"EnableFG", BoolToString(m_settings.enableFG).c_str(), m_filename.c_str());
}

const RXNSettings& RXNConfig::GetSettings() const {
    return m_settings;
}

void RXNConfig::UpdateSettings(const RXNSettings& new_settings) {
    m_settings = new_settings;
}

std::wstring RXNConfig::BoolToString(bool value) {
    return value ? L"1" : L"0";
}
