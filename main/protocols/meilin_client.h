#ifndef _MEILIN_CLIENT_H_
#define _MEILIN_CLIENT_H_

#include <string>
#include <vector>
#include <utility>
#include <esp_err.h>

/**
 * MeiLin Client - Communicate with MeiLin Python Backend
 * 
 * This client handles:
 * - Wake word event notification
 * - Chat message exchange  
 * - Audio file download from backend
 * - PUBLIC RAG API: Query knowledge base (read-only, with API key)
 * 
 * Hybrid Mode (ESP-only users):
 * - Use MeiLin Public RAG API for knowledge base
 * - Use XiaoZhi Cloud for LLM/TTS (free)
 */
class MeiLinClient {
public:
    /**
     * Constructor
     * @param backend_url MeiLin backend URL (e.g., https://meilin.your-domain.com)
     * @param device_id Unique device identifier
     */
    MeiLinClient(const std::string& backend_url, const std::string& device_id);
    
    /**
     * Destructor
     */
    ~MeiLinClient();
    
    // ============================================================
    // PUBLIC RAG API (Read-only, requires API key)
    // For ESP-only users who want MeiLin knowledge + XiaoZhi LLM
    // ============================================================
    
    /**
     * Register device to get API key
     * Call this once per device, save the API key in NVS
     * @param device_name Optional device name
     * @return API key string, empty if failed
     */
    std::string RegisterDevice(const std::string& device_name = "");
    
    /**
     * Set API key (load from NVS or manual set)
     * @param api_key API key from RegisterDevice
     */
    void SetApiKey(const std::string& api_key) { api_key_ = api_key; }
    
    /**
     * Get current API key
     */
    const std::string& GetApiKey() const { return api_key_; }
    
    /**
     * Query MeiLin knowledge base (PUBLIC API)
     * Returns relevant context to enhance XiaoZhi responses
     * @param query User's question
     * @param top_k Number of results (max 5)
     * @return Context string from knowledge base
     */
    std::string QueryRAG(const std::string& query, int top_k = 3);
    
    // ============================================================
    // PRIVATE API (Full access, for self-hosted users)
    // ============================================================
    
    /**
     * Destructor
     */
    ~MeiLinClient();
    
    /**
     * Send wake word detection event to backend
     * @param confidence Wake word detection confidence (0.0 - 1.0)
     * @return true if successful
     */
    bool SendWakeEvent(float confidence);
    
    /**
     * Send chat message to backend
     * @param message User message text
     * @param username Username
     * @return pair of (response_text, audio_url)
     */
    std::pair<std::string, std::string> SendChat(
        const std::string& message, 
        const std::string& username
    );
    
    /**
     * Send voice command to backend
     * @param command Voice command text
     * @param username Username
     * @return pair of (response_text, audio_url)
     */
    std::pair<std::string, std::string> SendCommand(
        const std::string& command,
        const std::string& username
    );
    
    /**
     * Download audio file from backend
     * @param audio_url Audio file URL
     * @return Audio data as byte vector
     */
    std::vector<uint8_t> DownloadAudio(const std::string& audio_url);
    
    /**
     * Check if backend is online
     * @return true if backend responds
     */
    bool CheckHealth();
    
    /**
     * Set backend URL
     */
    void SetBackendUrl(const std::string& url) { backend_url_ = url; }
    
    /**
     * Get backend URL
     */
    const std::string& GetBackendUrl() const { return backend_url_; }
    
    /**
     * Set device ID
     */
    void SetDeviceId(const std::string& id) { device_id_ = id; }
    
    /**
     * Get device ID
     */
    const std::string& GetDeviceId() const { return device_id_; }

private:
    std::string backend_url_;
    std::string device_id_;
    std::string api_key_;  // For public RAG API
    
    /**
     * Make HTTP POST request with JSON payload
     * @param endpoint API endpoint (e.g., /api/wake)
     * @param json_payload JSON string
     * @param response_buffer Buffer to store response
     * @param buffer_size Buffer size
     * @return HTTP status code
     */
    int HttpPost(
        const std::string& endpoint,
        const char* json_payload,
        char* response_buffer,
        size_t buffer_size
    );
    
    /**
     * Make HTTP GET request
     * @param url Full URL
     * @param data_buffer Buffer to store binary data
     * @return true if successful
     */
    bool HttpGet(const std::string& url, std::vector<uint8_t>& data_buffer);
    
    /**
     * Make HTTP POST request with API key header
     * For Public RAG API
     */
    int HttpPostWithApiKey(
        const std::string& endpoint,
        const char* json_payload,
        char* response_buffer,
        size_t buffer_size
    );
    
    /**
     * Get current timestamp in ISO8601 format
     * @return Timestamp string
     */
    std::string GetTimestamp();
};

#endif // _MEILIN_CLIENT_H_
