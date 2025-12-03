#ifndef IOT_SETTINGS_H
#define IOT_SETTINGS_H

#include "settings.h"
#include <string>

/**
 * @brief IoT Settings Manager
 * 
 * Manages MeiLin IoT Hybrid Mode settings in NVS.
 * Settings can be configured via:
 * 1. menuconfig (Kconfig defaults)
 * 2. Telegram Bot (runtime update)
 * 3. Web interface (if available)
 */
class IoTSettings {
public:
    static IoTSettings& GetInstance() {
        static IoTSettings instance;
        return instance;
    }

    // Delete copy constructor and assignment
    IoTSettings(const IoTSettings&) = delete;
    IoTSettings& operator=(const IoTSettings&) = delete;

    /**
     * @brief Initialize settings from NVS or Kconfig defaults
     */
    void Initialize();

    /**
     * @brief Check if IoT Hybrid Mode is enabled
     */
    bool IsEnabled() const { return enabled_; }

    /**
     * @brief Get MeiLin server URL
     */
    std::string GetServerUrl() const { return server_url_; }

    /**
     * @brief Get API key
     */
    std::string GetApiKey() const { return api_key_; }

    /**
     * @brief Get request timeout in ms
     */
    int GetTimeoutMs() const { return timeout_ms_; }

    /**
     * @brief Check if TTS response is enabled
     */
    bool IsTtsEnabled() const { return tts_enabled_; }

    /**
     * @brief Check if fallback to XiaoZhi is enabled
     */
    bool IsFallbackEnabled() const { return fallback_enabled_; }

    // Setters (save to NVS)
    void SetEnabled(bool enabled);
    void SetServerUrl(const std::string& url);
    void SetApiKey(const std::string& key);
    void SetTimeoutMs(int timeout);
    void SetTtsEnabled(bool enabled);
    void SetFallbackEnabled(bool enabled);

    /**
     * @brief Check if settings are valid for IoT operation
     */
    bool IsConfigured() const {
        return enabled_ && !server_url_.empty() && !api_key_.empty();
    }

    /**
     * @brief Print current settings to log
     */
    void PrintSettings() const;

private:
    IoTSettings() = default;
    ~IoTSettings() = default;

    bool enabled_ = false;
    std::string server_url_;
    std::string api_key_;
    int timeout_ms_ = 5000;
    bool tts_enabled_ = true;
    bool fallback_enabled_ = true;

    void LoadFromNvs();
    void LoadFromKconfig();
};

#endif // IOT_SETTINGS_H
