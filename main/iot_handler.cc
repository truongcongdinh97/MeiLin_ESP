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
        }
        last_health_check_ = now;
    }
    
    ESP_LOGI(TAG, "Checking IoT command: %s", text.c_str());
    
    // Check and execute IoT command with retry
    IoTExecuteResult result;
    bool is_iot = false;
    
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        is_iot = controller_->HandleIfIoTCommand(text, result);
        
        if (!is_iot) {
            // Not an IoT command, no need to retry
            ESP_LOGD(TAG, "Not an IoT command, forwarding to XiaoZhi");
            return false;
        }
        
        if (result.success) {
            // Success, break out of retry loop
            break;
        }
        
        // Failed, retry if not last attempt
        if (retry < MAX_RETRIES - 1) {
            ESP_LOGW(TAG, "IoT command failed, retry %d/%d: %s", 
                     retry + 1, MAX_RETRIES, result.error_message.c_str());
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        }
    }
    
    // Store result
    last_result_ = result;
    
    if (result.success) {
        ESP_LOGI(TAG, "IoT command executed successfully: %s", result.response_text.c_str());
        
        // Display the response
        ShowMessage("user", text);
        ShowMessage("assistant", result.response_text);
        
        // Play TTS if available and enabled
        if (settings.IsTtsEnabled()) {
            if (!result.audio_url.empty()) {
                PlayTts(result.response_text, result.audio_url);
            } else {
                // No audio URL, use local TTS fallback
                ESP_LOGW(TAG, "No audio URL provided, using text-only response");
            }
        }
    } else {
        ESP_LOGE(TAG, "IoT command failed after %d retries: %s", 
                 MAX_RETRIES, result.error_message.c_str());
        
        // Check if should fallback to XiaoZhi
        if (settings.IsFallbackEnabled()) {
            ESP_LOGI(TAG, "Fallback enabled, forwarding to XiaoZhi");
            return false;
        }
        
        // Show error message with user-friendly text
        std::string error_msg = "Không thể thực hiện lệnh: " + result.error_message;
        ShowMessage("assistant", error_msg);
    }
    
    return true;  // IoT was handled (successfully or not)
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
