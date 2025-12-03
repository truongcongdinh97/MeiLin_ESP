#ifndef IOT_CONTROLLER_H
#define IOT_CONTROLLER_H

#include <string>
#include <vector>
#include <esp_http_client.h>
#include <cJSON.h>

#define IOT_TAG "IoTController"

/**
 * @brief IoT check result structure
 */
struct IoTCheckResult {
    bool is_iot_command;        // Whether the text is an IoT command
    int device_id;              // Device ID to control (if is_iot_command)
    int action_id;              // Action ID to execute (if is_iot_command)
    std::string device_name;    // Device name for logging
    std::string action_name;    // Action name for logging
};

/**
 * @brief IoT execute result structure
 */
struct IoTExecuteResult {
    bool success;               // Whether execution was successful
    std::string response_text;  // Text response from server
    std::string audio_url;      // URL to TTS audio (if available)
    std::string error_message;  // Error message (if failed)
};

/**
 * @brief IoT Controller for Hybrid Mode
 * 
 * This class handles IoT command detection and execution.
 * It allows XiaoZhi firmware to use MeiLin's IoT features
 * while still forwarding non-IoT commands to XiaoZhi Cloud.
 * 
 * Flow:
 * 1. ESP32 receives voice input -> STT
 * 2. Call CheckIoTCommand() with recognized text
 * 3. If is_iot_command == true:
 *    - Call ExecuteIoTCommand() 
 *    - Play TTS response
 *    - DO NOT forward to XiaoZhi Cloud
 * 4. If is_iot_command == false:
 *    - Forward to XiaoZhi Cloud as normal
 */
class IoTController {
public:
    /**
     * @brief Constructor
     * @param meilin_server MeiLin server URL (e.g., "http://192.168.1.227:5000")
     * @param api_key Device API key (from Telegram Bot registration)
     */
    IoTController(const std::string& meilin_server, const std::string& api_key);
    ~IoTController();

    /**
     * @brief Check if text is an IoT command
     * @param text Recognized text from STT
     * @return IoTCheckResult with is_iot_command flag and device/action IDs
     */
    IoTCheckResult CheckIoTCommand(const std::string& text);

    /**
     * @brief Execute IoT command
     * @param text Original text
     * @param device_id Device ID from CheckIoTCommand
     * @param action_id Action ID from CheckIoTCommand
     * @return IoTExecuteResult with success flag, response text, and audio URL
     */
    IoTExecuteResult ExecuteIoTCommand(
        const std::string& text,
        int device_id,
        int action_id
    );

    /**
     * @brief Combined check and execute
     * @param text Recognized text from STT
     * @param out_result Output parameter for execute result (if IoT command)
     * @return true if this was an IoT command (handled), false if should forward to XiaoZhi
     */
    bool HandleIfIoTCommand(const std::string& text, IoTExecuteResult& out_result);

    /**
     * @brief Get list of IoT devices for this user
     * @return JSON string with device list
     */
    std::string GetDeviceList();

    /**
     * @brief Check if IoT server is reachable
     * @return true if server responds to health check
     */
    bool CheckServerHealth();

    // Configuration
    void SetServer(const std::string& server) { meilin_server_ = server; }
    void SetApiKey(const std::string& key) { api_key_ = key; }
    std::string GetServer() const { return meilin_server_; }
    bool IsConfigured() const { return !meilin_server_.empty() && !api_key_.empty(); }

private:
    std::string meilin_server_;
    std::string api_key_;

    // HTTP helper
    int HttpPost(
        const std::string& endpoint,
        const char* json_payload,
        char* response_buffer,
        size_t buffer_size
    );
};

#endif // IOT_CONTROLLER_H
