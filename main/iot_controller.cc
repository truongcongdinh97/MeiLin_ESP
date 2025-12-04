#include "iot_controller.h"

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include <cJSON.h>

#define MAX_RESPONSE_BUFFER 4096

// HTTP event handler
static esp_err_t _iot_http_event_handler(esp_http_client_event_t *evt) {
    static int output_len = 0;
    
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(IOT_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(IOT_TAG, "HTTP_EVENT_ON_CONNECTED");
            output_len = 0;
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(IOT_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (evt->user_data) {
                    int copy_len = evt->data_len;
                    if (output_len + copy_len > MAX_RESPONSE_BUFFER - 1) {
                        copy_len = MAX_RESPONSE_BUFFER - 1 - output_len;
                    }
                    if (copy_len > 0) {
                        memcpy((char*)evt->user_data + output_len, evt->data, copy_len);
                        output_len += copy_len;
                        ((char*)evt->user_data)[output_len] = '\0';
                    }
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(IOT_TAG, "HTTP_EVENT_ON_FINISH");
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(IOT_TAG, "HTTP_EVENT_DISCONNECTED");
            output_len = 0;
            break;
        default:
            break;
    }
    return ESP_OK;
}

IoTController::IoTController(const std::string& meilin_server, const std::string& api_key)
    : meilin_server_(meilin_server), api_key_(api_key) {
    ESP_LOGI(IOT_TAG, "IoT Controller initialized: server=%s", meilin_server_.c_str());
}

IoTController::~IoTController() {
    ESP_LOGI(IOT_TAG, "IoT Controller destroyed");
}

IoTCheckResult IoTController::CheckIoTCommand(const std::string& text) {
    IoTCheckResult result = {false, -1, -1, "", ""};
    
    if (!IsConfigured()) {
        ESP_LOGW(IOT_TAG, "IoT Controller not configured");
        return result;
    }
    
    // Build JSON payload
    char json_buffer[1024];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"text\":\"%s\",\"user_id\":\"%s\"}",
             text.c_str(),
             api_key_.c_str());
    
    char response[MAX_RESPONSE_BUFFER] = {0};
    int status = HttpPost("/iot/check", json_buffer, response, sizeof(response));
    
    if (status != 200) {
        ESP_LOGE(IOT_TAG, "IoT check failed, status: %d", status);
        return result;
    }
    
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL) {
        ESP_LOGE(IOT_TAG, "Failed to parse IoT check response");
        return result;
    }
    
    // Extract is_iot_command
    cJSON *is_iot = cJSON_GetObjectItem(json, "is_iot_command");
    if (is_iot && cJSON_IsBool(is_iot)) {
        result.is_iot_command = cJSON_IsTrue(is_iot);
    }
    
    if (result.is_iot_command) {
        // Extract device and action IDs
        cJSON *device_id = cJSON_GetObjectItem(json, "device_id");
        if (device_id && cJSON_IsNumber(device_id)) {
            result.device_id = device_id->valueint;
        }
        
        cJSON *action_id = cJSON_GetObjectItem(json, "action_id");
        if (action_id && cJSON_IsNumber(action_id)) {
            result.action_id = action_id->valueint;
        }
        
        cJSON *device_name = cJSON_GetObjectItem(json, "device_name");
        if (device_name && cJSON_IsString(device_name)) {
            result.device_name = device_name->valuestring;
        }
        
        cJSON *action_name = cJSON_GetObjectItem(json, "action_name");
        if (action_name && cJSON_IsString(action_name)) {
            result.action_name = action_name->valuestring;
        }
        
        ESP_LOGI(IOT_TAG, "IoT command detected: device=%s, action=%s",
                 result.device_name.c_str(), result.action_name.c_str());
    } else {
        ESP_LOGD(IOT_TAG, "Not an IoT command: %s", text.c_str());
    }
    
    cJSON_Delete(json);
    return result;
}

IoTExecuteResult IoTController::ExecuteIoTCommand(
    const std::string& text,
    int device_id,
    int action_id) {
    
    IoTExecuteResult result = {false, "", "", ""};
    
    if (!IsConfigured()) {
        result.error_message = "Hệ thống IoT chưa được cấu hình";
        ESP_LOGW(IOT_TAG, "%s", result.error_message.c_str());
        return result;
    }
    
    // Build JSON payload
    char json_buffer[1024];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"text\":\"%s\",\"device_id\":%d,\"action_id\":%d,\"user_id\":\"%s\"}",
             text.c_str(),
             device_id,
             action_id,
             api_key_.c_str());
    
    char response[MAX_RESPONSE_BUFFER] = {0};
    int status = HttpPost("/iot/execute", json_buffer, response, sizeof(response));
    
    if (status == 0) {
        result.error_message = "Không thể kết nối tới máy chủ IoT";
        ESP_LOGE(IOT_TAG, "IoT execute failed: connection error");
        return result;
    }
    
    if (status != 200) {
        result.error_message = "Máy chủ IoT phản hồi lỗi: " + std::to_string(status);
        ESP_LOGE(IOT_TAG, "IoT execute failed, status: %d", status);
        return result;
    }
    
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL) {
        result.error_message = "Không thể đọc phản hồi từ máy chủ";
        ESP_LOGE(IOT_TAG, "Failed to parse IoT execute response");
        return result;
    }
    
    // Extract success
    cJSON *success = cJSON_GetObjectItem(json, "success");
    if (success && cJSON_IsBool(success)) {
        result.success = cJSON_IsTrue(success);
    }
    
    // Extract response text
    cJSON *response_text = cJSON_GetObjectItem(json, "response");
    if (response_text && cJSON_IsString(response_text)) {
        result.response_text = response_text->valuestring;
    }
    
    // Extract audio URL (optional)
    cJSON *audio_url = cJSON_GetObjectItem(json, "audio_url");
    if (audio_url && cJSON_IsString(audio_url)) {
        result.audio_url = audio_url->valuestring;
    }
    
    // Extract error message if failed
    if (!result.success) {
        cJSON *error = cJSON_GetObjectItem(json, "error");
        if (error && cJSON_IsString(error)) {
            result.error_message = error->valuestring;
        }
    }
    
    ESP_LOGI(IOT_TAG, "IoT execute result: success=%d, response=%s",
             result.success, result.response_text.c_str());
    
    cJSON_Delete(json);
    return result;
}

bool IoTController::HandleIfIoTCommand(const std::string& text, IoTExecuteResult& out_result) {
    // Step 1: Check if this is an IoT command
    IoTCheckResult check = CheckIoTCommand(text);
    
    if (!check.is_iot_command) {
        // Not an IoT command, caller should forward to XiaoZhi
        return false;
    }
    
    // Step 2: Execute the IoT command
    out_result = ExecuteIoTCommand(text, check.device_id, check.action_id);
    
    // Return true to indicate this was handled as IoT
    // (even if execution failed, we don't want to forward to XiaoZhi)
    return true;
}

std::string IoTController::GetDeviceList() {
    if (!IsConfigured()) {
        ESP_LOGW(IOT_TAG, "IoT Controller not configured");
        return "{}";
    }
    
    std::string url = meilin_server_ + "/iot/devices?user_id=" + api_key_;
    
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.timeout_ms = 10000;
    
    char response[MAX_RESPONSE_BUFFER] = {0};
    config.event_handler = _iot_http_event_handler;
    config.user_data = response;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_http_client_set_header(client, "X-API-Key", api_key_.c_str());
    
    esp_err_t err = esp_http_client_perform(client);
    int status_code = 0;
    
    if (err == ESP_OK) {
        status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(IOT_TAG, "Get devices status = %d", status_code);
    } else {
        ESP_LOGE(IOT_TAG, "Get devices failed: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
    
    if (status_code == 200) {
        return std::string(response);
    }
    
    return "{}";
}

bool IoTController::CheckServerHealth() {
    if (meilin_server_.empty()) {
        return false;
    }
    
    std::string url = meilin_server_ + "/health";
    
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.timeout_ms = 5000;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    int status_code = 0;
    
    if (err == ESP_OK) {
        status_code = esp_http_client_get_status_code(client);
    }
    
    esp_http_client_cleanup(client);
    
    bool healthy = (status_code == 200);
    ESP_LOGI(IOT_TAG, "IoT server health: %s", healthy ? "OK" : "FAILED");
    
    return healthy;
}

IoTExecuteResult IoTController::SendChatToMeiLin(const std::string& text) {
    IoTExecuteResult result = {false, "", "", ""};
    
    if (!IsConfigured()) {
        result.error_message = "MeiLin chưa được cấu hình";
        ESP_LOGW(IOT_TAG, "%s", result.error_message.c_str());
        return result;
    }
    
    // Build JSON payload for /esp/chat endpoint
    // api_key_ contains the device_api_key (meilin_dev_xxxx)
    char json_buffer[2048];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"message\":\"%s\",\"device_api_key\":\"%s\"}",
             text.c_str(),
             api_key_.c_str());
    
    char response[MAX_RESPONSE_BUFFER] = {0};
    int status = HttpPost("/esp/chat", json_buffer, response, sizeof(response));
    
    if (status == 0) {
        result.error_message = "Không thể kết nối tới MeiLin server";
        ESP_LOGE(IOT_TAG, "MeiLin chat failed: connection error");
        return result;
    }
    
    if (status != 200) {
        result.error_message = "MeiLin phản hồi lỗi: " + std::to_string(status);
        ESP_LOGE(IOT_TAG, "MeiLin chat failed, status: %d", status);
        return result;
    }
    
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL) {
        result.error_message = "Không thể đọc phản hồi từ MeiLin";
        ESP_LOGE(IOT_TAG, "Failed to parse MeiLin chat response");
        return result;
    }
    
    result.success = true;
    
    // Extract response text
    cJSON *response_text = cJSON_GetObjectItem(json, "response");
    if (response_text && cJSON_IsString(response_text)) {
        result.response_text = response_text->valuestring;
    }
    
    // Extract audio URL (optional)
    cJSON *audio_url = cJSON_GetObjectItem(json, "audio_url");
    if (audio_url && cJSON_IsString(audio_url)) {
        result.audio_url = audio_url->valuestring;
    }
    
    ESP_LOGI(IOT_TAG, "MeiLin chat result: response=%s, audio=%s",
             result.response_text.c_str(), 
             result.audio_url.empty() ? "(none)" : result.audio_url.c_str());
    
    cJSON_Delete(json);
    return result;
}

int IoTController::HttpPost(
    const std::string& endpoint,
    const char* json_payload,
    char* response_buffer,
    size_t buffer_size) {
    
    std::string url = meilin_server_ + endpoint;
    
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = _iot_http_event_handler;
    config.user_data = response_buffer;
    config.timeout_ms = 10000;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "X-API-Key", api_key_.c_str());
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));
    
    esp_err_t err = esp_http_client_perform(client);
    int status_code = 0;
    
    if (err == ESP_OK) {
        status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(IOT_TAG, "HTTP POST %s status = %d", endpoint.c_str(), status_code);
    } else {
        ESP_LOGE(IOT_TAG, "HTTP POST %s failed: %s", endpoint.c_str(), esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
    return status_code;
}
