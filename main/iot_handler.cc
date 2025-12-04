#include "iot_handler.h"
#include "iot_controller.h"
#include "iot_settings.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "IoTHandler"

// Retry configuration
static constexpr int MAX_RETRIES = 3;
static constexpr int RETRY_DELAY_MS = 500;
static constexpr int HEALTH_CHECK_INTERVAL_MS = 30000;  // 30 seconds

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
    
    // Check server health with retry
    bool server_healthy = false;
    for (int i = 0; i < MAX_RETRIES; i++) {
        if (controller_->CheckServerHealth()) {
            server_healthy = true;
            ESP_LOGI(TAG, "IoT server health check passed");
            break;
        }
        ESP_LOGW(TAG, "IoT server health check failed, retry %d/%d", i + 1, MAX_RETRIES);
        if (i < MAX_RETRIES - 1) {
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        }
    }
    
    if (!server_healthy) {
        ESP_LOGW(TAG, "IoT server not reachable after %d retries, will retry on demand", MAX_RETRIES);
        // Don't disable - server might come online later
    }
    
    available_ = true;
    last_health_check_ = xTaskGetTickCount();
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
    
    // Periodic health check
    TickType_t now = xTaskGetTickCount();
    if ((now - last_health_check_) > pdMS_TO_TICKS(HEALTH_CHECK_INTERVAL_MS)) {
        if (!controller_->CheckServerHealth()) {
            ESP_LOGW(TAG, "IoT server became unreachable");
            // If MeiLin server is down, fallback to XiaoZhi
            if (settings.IsFallbackEnabled()) {
                ESP_LOGI(TAG, "MeiLin unreachable, fallback to XiaoZhi");
                return false;
            }
        }
        last_health_check_ = now;
    }
    
    ESP_LOGI(TAG, "Processing with MeiLin: %s", text.c_str());
    
    // First, check if this is an IoT command
    IoTExecuteResult result;
    bool is_iot = false;
    
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        is_iot = controller_->HandleIfIoTCommand(text, result);
        
        if (is_iot && result.success) {
            // IoT command executed successfully
            break;
        }
        
        if (is_iot && !result.success && retry < MAX_RETRIES - 1) {
            // IoT command failed, retry
            ESP_LOGW(TAG, "IoT command failed, retry %d/%d: %s", 
                     retry + 1, MAX_RETRIES, result.error_message.c_str());
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
            continue;
        }
        
        // Not an IoT command OR IoT failed after retries
        break;
    }
    
    if (is_iot) {
        // This was an IoT command
        last_result_ = result;
        
        if (result.success) {
            ESP_LOGI(TAG, "IoT command executed: %s", result.response_text.c_str());
            ShowMessage("user", text);
            ShowMessage("assistant", result.response_text);
            
            if (settings.IsTtsEnabled() && !result.audio_url.empty()) {
                PlayTts(result.response_text, result.audio_url);
            }
        } else {
            ESP_LOGE(TAG, "IoT command failed: %s", result.error_message.c_str());
            std::string error_msg = "Không thể thực hiện lệnh: " + result.error_message;
            ShowMessage("assistant", error_msg);
        }
        
        return true;
    }
    
    // NOT an IoT command - send to MeiLin for regular chat
    ESP_LOGI(TAG, "Not IoT, sending to MeiLin chat: %s", text.c_str());
    
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        result = controller_->SendChatToMeiLin(text);
        
        if (result.success) {
            break;
        }
        
        if (retry < MAX_RETRIES - 1) {
            ESP_LOGW(TAG, "MeiLin chat failed, retry %d/%d: %s", 
                     retry + 1, MAX_RETRIES, result.error_message.c_str());
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        }
    }
    
    last_result_ = result;
    
    if (result.success) {
        ESP_LOGI(TAG, "MeiLin response: %s", result.response_text.c_str());
        
        ShowMessage("user", text);
        ShowMessage("assistant", result.response_text);
        
        if (settings.IsTtsEnabled() && !result.audio_url.empty()) {
            PlayTts(result.response_text, result.audio_url);
        }
        
        return true;
    }
    
    // MeiLin chat failed
    ESP_LOGE(TAG, "MeiLin chat failed: %s", result.error_message.c_str());
    
    // Check if should fallback to XiaoZhi
    if (settings.IsFallbackEnabled()) {
        ESP_LOGI(TAG, "Fallback enabled, forwarding to XiaoZhi");
        return false;  // Let XiaoZhi handle it
    }
    
    // Show error to user
    std::string error_msg = "Xin lỗi, MeiLin không thể trả lời: " + result.error_message;
    ShowMessage("assistant", error_msg);
    
    return true;  // Handled (with error)
}

void IoTHandler::RefreshDeviceList() {
    if (!available_ || !controller_) {
        ESP_LOGW(TAG, "IoT not available");
        return;
    }
    
    std::string devices = controller_->GetDeviceList();
    if (devices.empty() || devices == "{}") {
        ESP_LOGW(TAG, "No IoT devices configured");
    } else {
        ESP_LOGI(TAG, "Device list refreshed: %s", devices.c_str());
    }
}

bool IoTHandler::CheckServerHealth() {
    if (!available_ || !controller_) {
        return false;
    }
    
    bool healthy = controller_->CheckServerHealth();
    last_health_check_ = xTaskGetTickCount();
    
    if (healthy) {
        ESP_LOGI(TAG, "IoT server is healthy");
    } else {
        ESP_LOGW(TAG, "IoT server is not responding");
    }
    
    return healthy;
}

void IoTHandler::ShowMessage(const std::string& role, const std::string& message) {
    if (message.empty()) {
        ESP_LOGW(TAG, "Empty message for role: %s", role.c_str());
        return;
    }
    
    if (display_callback_) {
        display_callback_(role, message);
    } else {
        ESP_LOGI(TAG, "[%s] %s", role.c_str(), message.c_str());
    }
}

void IoTHandler::PlayTts(const std::string& text, const std::string& audio_url) {
    if (text.empty()) {
        ESP_LOGW(TAG, "Empty TTS text");
        return;
    }
    
    if (tts_callback_) {
        tts_callback_(text, audio_url);
    } else {
        ESP_LOGI(TAG, "TTS (no callback): %s", text.c_str());
    }
}
