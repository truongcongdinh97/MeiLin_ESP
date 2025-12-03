#ifndef MEILIN_CLIENT_H
#define MEILIN_CLIENT_H

#include <string>
#include <vector>
#include <esp_http_client.h>
#include <cJSON.h>

#define TAG "MeiLinClient"
#define MAX_HTTP_OUTPUT_BUFFER 8192

class MeiLinClient {
private:
    std::string backend_url_;
    std::string device_id_;

    // HTTP event handler for POST requests
    static esp_err_t HttpEventHandler(esp_http_client_event_t *evt);

public:
    MeiLinClient(const std::string& backend_url, const std::string& device_id);
    ~MeiLinClient();

    // Wake word detection
    bool SendWakeEvent(float confidence = 0.95f);

    // Chat communication
    std::pair<std::string, std::string> SendChatMessage(
        const std::string& message,
        const std::string& username = "user");

    // Voice commands
    std::pair<std::string, std::string> SendCommand(
        const std::string& command,
        const std::string& username = "user");

    // Audio download
    std::vector<uint8_t> DownloadAudio(const std::string& audio_url);

    // Health check
    bool CheckHealth();

    // Configuration
    void SetBackendUrl(const std::string& url) { backend_url_ = url; }
    void SetDeviceId(const std::string& id) { device_id_ = id; }
    std::string GetBackendUrl() const { return backend_url_; }
    std::string GetDeviceId() const { return device_id_; }

private:
    // HTTP methods
    int HttpPost(
        const std::string& endpoint,
        const char* json_payload,
        char* response_buffer,
        size_t buffer_size);

    bool HttpGet(const std::string& url, std::vector<uint8_t>& data_buffer);

    // Utility methods
    std::string GetTimestamp();
    std::string GenerateDeviceId();
};

#endif // MEILIN_CLIENT_H
