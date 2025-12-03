#include "iot_settings.h"
#include "settings.h"

#include <esp_log.h>
#include <sdkconfig.h>

#define TAG "IoTSettings"

void IoTSettings::Initialize() {
    // First load Kconfig defaults
    LoadFromKconfig();
    
    // Then override with NVS values if present
    LoadFromNvs();
    
    PrintSettings();
}

void IoTSettings::LoadFromKconfig() {
#ifdef CONFIG_MEILIN_IOT_ENABLED
    enabled_ = CONFIG_MEILIN_IOT_ENABLED;
#else
    enabled_ = false;
#endif

#ifdef CONFIG_MEILIN_IOT_SERVER
    server_url_ = CONFIG_MEILIN_IOT_SERVER;
#else
    server_url_ = "";
#endif

#ifdef CONFIG_MEILIN_IOT_API_KEY
    api_key_ = CONFIG_MEILIN_IOT_API_KEY;
#else
    api_key_ = "";
#endif

#ifdef CONFIG_MEILIN_IOT_TIMEOUT_MS
    timeout_ms_ = CONFIG_MEILIN_IOT_TIMEOUT_MS;
#else
    timeout_ms_ = 5000;
#endif

#ifdef CONFIG_MEILIN_IOT_ENABLE_TTS
    tts_enabled_ = CONFIG_MEILIN_IOT_ENABLE_TTS;
#else
    tts_enabled_ = true;
#endif

#ifdef CONFIG_MEILIN_IOT_FALLBACK_TO_XIAOZHI
    fallback_enabled_ = CONFIG_MEILIN_IOT_FALLBACK_TO_XIAOZHI;
#else
    fallback_enabled_ = true;
#endif

    ESP_LOGI(TAG, "Loaded Kconfig defaults");
}

void IoTSettings::LoadFromNvs() {
    Settings settings("iot", false);
    
    // Check if NVS has any IoT settings
    std::string nvs_server = settings.GetString("server");
    if (!nvs_server.empty()) {
        server_url_ = nvs_server;
    }
    
    std::string nvs_api_key = settings.GetString("api_key");
    if (!nvs_api_key.empty()) {
        api_key_ = nvs_api_key;
    }
    
    // Boolean values - check if key exists by using a sentinel default
    int nvs_enabled = settings.GetInt("enabled", -1);
    if (nvs_enabled != -1) {
        enabled_ = (nvs_enabled == 1);
    }
    
    int nvs_timeout = settings.GetInt("timeout", -1);
    if (nvs_timeout != -1) {
        timeout_ms_ = nvs_timeout;
    }
    
    int nvs_tts = settings.GetInt("tts", -1);
    if (nvs_tts != -1) {
        tts_enabled_ = (nvs_tts == 1);
    }
    
    int nvs_fallback = settings.GetInt("fallback", -1);
    if (nvs_fallback != -1) {
        fallback_enabled_ = (nvs_fallback == 1);
    }
    
    ESP_LOGI(TAG, "Loaded NVS overrides");
}

void IoTSettings::SetEnabled(bool enabled) {
    enabled_ = enabled;
    Settings settings("iot", true);
    settings.SetInt("enabled", enabled ? 1 : 0);
    ESP_LOGI(TAG, "IoT enabled: %s", enabled ? "true" : "false");
}

void IoTSettings::SetServerUrl(const std::string& url) {
    server_url_ = url;
    Settings settings("iot", true);
    settings.SetString("server", url);
    ESP_LOGI(TAG, "IoT server URL: %s", url.c_str());
}

void IoTSettings::SetApiKey(const std::string& key) {
    api_key_ = key;
    Settings settings("iot", true);
    settings.SetString("api_key", key);
    ESP_LOGI(TAG, "IoT API key updated (length: %zu)", key.length());
}

void IoTSettings::SetTimeoutMs(int timeout) {
    timeout_ms_ = timeout;
    Settings settings("iot", true);
    settings.SetInt("timeout", timeout);
    ESP_LOGI(TAG, "IoT timeout: %d ms", timeout);
}

void IoTSettings::SetTtsEnabled(bool enabled) {
    tts_enabled_ = enabled;
    Settings settings("iot", true);
    settings.SetInt("tts", enabled ? 1 : 0);
    ESP_LOGI(TAG, "IoT TTS: %s", enabled ? "enabled" : "disabled");
}

void IoTSettings::SetFallbackEnabled(bool enabled) {
    fallback_enabled_ = enabled;
    Settings settings("iot", true);
    settings.SetInt("fallback", enabled ? 1 : 0);
    ESP_LOGI(TAG, "IoT fallback: %s", enabled ? "enabled" : "disabled");
}

void IoTSettings::PrintSettings() const {
    ESP_LOGI(TAG, "=== MeiLin IoT Settings ===");
    ESP_LOGI(TAG, "  Enabled: %s", enabled_ ? "true" : "false");
    ESP_LOGI(TAG, "  Server: %s", server_url_.c_str());
    ESP_LOGI(TAG, "  API Key: %s", api_key_.empty() ? "(not set)" : "(set)");
    ESP_LOGI(TAG, "  Timeout: %d ms", timeout_ms_);
    ESP_LOGI(TAG, "  TTS: %s", tts_enabled_ ? "enabled" : "disabled");
    ESP_LOGI(TAG, "  Fallback: %s", fallback_enabled_ ? "enabled" : "disabled");
    ESP_LOGI(TAG, "  Configured: %s", IsConfigured() ? "yes" : "no");
    ESP_LOGI(TAG, "===========================");
}
