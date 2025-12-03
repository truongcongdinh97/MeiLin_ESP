#include "iot_handler.h"
#include "iot_controller.h"
#include "iot_settings.h"

#include <esp_log.h>

#define TAG "IoTHandler"

void IoTHandler::Initialize() {
    auto& settings = IoTSettings::GetInstance();
    
    if (!settings.IsConfigured()) {
        ESP_LOGW(TAG, "IoT not configured, handler disabled");
        available_ = false;
        return;
    }
    
    // Create controller
    controller_ = std::make_unique<IoTController>(
        settings.GetServerUrl(),
        settings.GetApiKey()
    );
    
    // Check server health
    if (!controller_->CheckServerHealth()) {
        ESP_LOGW(TAG, "IoT server not reachable, handler may not work");
        // Don't disable - server might come online later
    }
    
    available_ = true;
    ESP_LOGI(TAG, "IoT Handler initialized successfully");
}

bool IoTHandler::HandleSttResult(const std::string& text) {
    if (!available_ || !controller_) {
        return false;
    }
    
    auto& settings = IoTSettings::GetInstance();
    if (!settings.IsEnabled()) {
        return false;
    }
    
    ESP_LOGI(TAG, "Checking IoT command: %s", text.c_str());
    
    // Check and execute IoT command
    IoTExecuteResult result;
    bool is_iot = controller_->HandleIfIoTCommand(text, result);
    
    if (!is_iot) {
        ESP_LOGD(TAG, "Not an IoT command, forwarding to XiaoZhi");
        return false;
    }
    
    // Store result
    last_result_ = result;
    
    if (result.success) {
        ESP_LOGI(TAG, "IoT command executed successfully: %s", result.response_text.c_str());
        
        // Display the response
        ShowMessage("user", text);
        ShowMessage("assistant", result.response_text);
        
        // Play TTS if available and enabled
        if (settings.IsTtsEnabled() && !result.audio_url.empty()) {
            PlayTts(result.response_text, result.audio_url);
        }
    } else {
        ESP_LOGE(TAG, "IoT command failed: %s", result.error_message.c_str());
        
        // Check if should fallback to XiaoZhi
        if (settings.IsFallbackEnabled()) {
            ESP_LOGI(TAG, "Fallback enabled, forwarding to XiaoZhi");
            return false;
        }
        
        // Show error message
        ShowMessage("assistant", result.error_message);
    }
    
    return true;  // IoT was handled (successfully or not)
}

void IoTHandler::RefreshDeviceList() {
    if (!available_ || !controller_) {
        ESP_LOGW(TAG, "IoT not available");
        return;
    }
    
    std::string devices = controller_->GetDeviceList();
    ESP_LOGI(TAG, "Device list: %s", devices.c_str());
}

bool IoTHandler::CheckServerHealth() {
    if (!available_ || !controller_) {
        return false;
    }
    
    return controller_->CheckServerHealth();
}

void IoTHandler::ShowMessage(const std::string& role, const std::string& message) {
    if (display_callback_) {
        display_callback_(role, message);
    } else {
        ESP_LOGI(TAG, "[%s] %s", role.c_str(), message.c_str());
    }
}

void IoTHandler::PlayTts(const std::string& text, const std::string& audio_url) {
    if (tts_callback_) {
        tts_callback_(text, audio_url);
    } else {
        ESP_LOGI(TAG, "TTS: %s (URL: %s)", text.c_str(), audio_url.c_str());
    }
}
