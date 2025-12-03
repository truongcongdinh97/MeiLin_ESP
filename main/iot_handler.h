#ifndef IOT_HANDLER_H
#define IOT_HANDLER_H

#include "iot_controller.h"
#include "iot_settings.h"
#include <string>
#include <memory>
#include <functional>

/**
 * @brief IoT Handler for XiaoZhi Hybrid Mode
 * 
 * This class integrates IoT control with XiaoZhi Assistant.
 * It hooks into the STT result processing to check for IoT commands.
 * 
 * Usage:
 *   1. Initialize once at startup
 *   2. Call HandleSttResult() when STT text is received
 *   3. If returns true, IoT command was handled (don't forward to XiaoZhi LLM)
 *   4. If returns false, proceed with normal XiaoZhi flow
 */
class IoTHandler {
public:
    static IoTHandler& GetInstance() {
        static IoTHandler instance;
        return instance;
    }

    // Delete copy constructor and assignment
    IoTHandler(const IoTHandler&) = delete;
    IoTHandler& operator=(const IoTHandler&) = delete;

    /**
     * @brief Initialize IoT handler
     * Must be called after IoTSettings is initialized
     */
    void Initialize();

    /**
     * @brief Check if IoT is available
     */
    bool IsAvailable() const { return available_; }

    /**
     * @brief Handle STT result
     * @param text Recognized text from XiaoZhi STT
     * @return true if this was an IoT command (handled), false otherwise
     */
    bool HandleSttResult(const std::string& text);

    /**
     * @brief Set callback for playing TTS audio
     * @param callback Function that takes audio URL and plays it
     */
    void SetTtsCallback(std::function<void(const std::string& text, const std::string& audio_url)> callback) {
        tts_callback_ = callback;
    }

    /**
     * @brief Set callback for display message
     * @param callback Function that displays a message
     */
    void SetDisplayCallback(std::function<void(const std::string& role, const std::string& message)> callback) {
        display_callback_ = callback;
    }

    /**
     * @brief Get last IoT result
     */
    const IoTExecuteResult& GetLastResult() const { return last_result_; }

    /**
     * @brief Refresh device list from server
     */
    void RefreshDeviceList();

    /**
     * @brief Check server health
     */
    bool CheckServerHealth();

private:
    IoTHandler() = default;
    ~IoTHandler() = default;

    bool available_ = false;
    std::unique_ptr<IoTController> controller_;
    IoTExecuteResult last_result_;

    std::function<void(const std::string& text, const std::string& audio_url)> tts_callback_;
    std::function<void(const std::string& role, const std::string& message)> display_callback_;

    void ShowMessage(const std::string& role, const std::string& message);
    void PlayTts(const std::string& text, const std::string& audio_url);
};

#endif // IOT_HANDLER_H
