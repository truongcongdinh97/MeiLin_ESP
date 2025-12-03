#include "meilin_client.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include <cJSON.h>

#define TAG "MeiLinClient"
#define MAX_HTTP_OUTPUT_BUFFER 8192

// HTTP event handler for POST requests
static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer;
    static int output_len;
    
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", 
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                    output_len += evt->data_len;
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            output_len = 0;
            break;
        default:
            break;
    }
    return ESP_OK;
}

MeiLinClient::MeiLinClient(const std::string& backend_url, const std::string& device_id) 
    : backend_url_(backend_url), device_id_(device_id) {
    ESP_LOGI(TAG, "MeiLin Client initialized: backend=%s, device_id=%s", 
             backend_url_.c_str(), device_id_.c_str());
}

MeiLinClient::~MeiLinClient() {
    ESP_LOGI(TAG, "MeiLin Client destroyed");
}

bool MeiLinClient::SendWakeEvent(float confidence) {
    char json_buffer[256];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"device_id\":\"%s\",\"timestamp\":\"%s\",\"confidence\":%.2f}",
             device_id_.c_str(),
             GetTimestamp().c_str(),
             confidence);
    
    char response[512] = {0};
    int status = HttpPost("/wake", json_buffer, response, sizeof(response));
    
    if (status == 200) {
        ESP_LOGI(TAG, "Wake event sent successfully, response: %s", response);
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to send wake event, status: %d", status);
        return false;
    }
}

std::pair<std::string, std::string> MeiLinClient::SendChat(
    const std::string& message,
    const std::string& username) {
    
    char json_buffer[2048];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"message\":\"%s\",\"username\":\"%s\",\"device_id\":\"%s\"}",
             message.c_str(),
             username.c_str(),
             device_id_.c_str());
    
    char response[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int status = HttpPost("/chat", json_buffer, response, sizeof(response));
    
    if (status != 200) {
        ESP_LOGE(TAG, "Failed to send chat message, status: %d", status);
        return {"", ""};
    }
    
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        return {"", ""};
    }
    
    std::string response_text;
    std::string audio_url;
    
    cJSON *response_item = cJSON_GetObjectItem(json, "response");
    if (response_item && cJSON_IsString(response_item)) {
        response_text = response_item->valuestring;
    }
    
    cJSON *audio_item = cJSON_GetObjectItem(json, "audio_url");
    if (audio_item && cJSON_IsString(audio_item)) {
        audio_url = audio_item->valuestring;
    }
    
    cJSON_Delete(json);
    
    ESP_LOGI(TAG, "Chat response: %s", response_text.c_str());
    ESP_LOGI(TAG, "Audio URL: %s", audio_url.c_str());
    
    return {response_text, audio_url};
}

std::pair<std::string, std::string> MeiLinClient::SendCommand(
    const std::string& command,
    const std::string& username) {
    
    char json_buffer[2048];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"command\":\"%s\",\"username\":\"%s\",\"device_id\":\"%s\"}",
             command.c_str(),
             username.c_str(),
             device_id_.c_str());
    
    char response[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int status = HttpPost("/command", json_buffer, response, sizeof(response));
    
    if (status != 200) {
        ESP_LOGE(TAG, "Failed to send command, status: %d", status);
        return {"", ""};
    }
    
    // Parse JSON response
    cJSON *json = cJSON_Parse(response);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        return {"", ""};
    }
    
    std::string response_text;
    std::string audio_url;
    
    cJSON *response_item = cJSON_GetObjectItem(json, "response");
    if (response_item && cJSON_IsString(response_item)) {
        response_text = response_item->valuestring;
    }
    
    cJSON *audio_item = cJSON_GetObjectItem(json, "audio_url");
    if (audio_item && cJSON_IsString(audio_item)) {
        audio_url = audio_item->valuestring;
    }
    
    cJSON_Delete(json);
    
    ESP_LOGI(TAG, "Command response: %s", response_text.c_str());
    ESP_LOGI(TAG, "Audio URL: %s", audio_url.c_str());
    
    return {response_text, audio_url};
}

std::vector<uint8_t> MeiLinClient::DownloadAudio(const std::string& audio_url) {
    std::vector<uint8_t> audio_data;
    
    if (!HttpGet(audio_url, audio_data)) {
        ESP_LOGE(TAG, "Failed to download audio from: %s", audio_url.c_str());
        return {};
    }
    
    ESP_LOGI(TAG, "Downloaded audio: %zu bytes", audio_data.size());
    return audio_data;
}

bool MeiLinClient::CheckHealth() {
    std::string health_url = backend_url_ + "/health";
    std::vector<uint8_t> dummy;
    return HttpGet(health_url, dummy);
}

int MeiLinClient::HttpPost(
    const std::string& endpoint,
    const char* json_payload,
    char* response_buffer,
    size_t buffer_size) {
    
    std::string url = backend_url_ + endpoint;
    
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = _http_event_handler;
    config.user_data = response_buffer;
    config.timeout_ms = 10000;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));
    
    esp_err_t err = esp_http_client_perform(client);
    int status_code = 0;
    
    if (err == ESP_OK) {
        status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                 status_code,
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
    return status_code;
}

bool MeiLinClient::HttpGet(const std::string& url, std::vector<uint8_t>& data_buffer) {
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.timeout_ms = 30000;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }
    
    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    
    if (status_code != 200) {
        ESP_LOGE(TAG, "HTTP GET failed with status: %d", status_code);
        esp_http_client_cleanup(client);
        return false;
    }
    
    if (content_length <= 0) {
        ESP_LOGE(TAG, "Invalid content length: %d", content_length);
        esp_http_client_cleanup(client);
        return false;
    }
    
    // Reserve space for audio data
    data_buffer.reserve(content_length);
    
    char buffer[1024];
    int read_len;
    
    while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
        data_buffer.insert(data_buffer.end(), buffer, buffer + read_len);
    }
    
    esp_http_client_cleanup(client);
    
    ESP_LOGI(TAG, "HTTP GET complete: %zu bytes downloaded", data_buffer.size());
    return true;
}

std::string MeiLinClient::GetTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    time_t now = tv.tv_sec;
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    
    // Add milliseconds
    char final_buffer[40];
    snprintf(final_buffer, sizeof(final_buffer), "%s.%03ldZ", 
             buffer, tv.tv_usec / 1000);
    
    return std::string(final_buffer);
}
