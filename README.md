# MeiLin ESP32 Firmware

**Firmware tương thích hoàn toàn với XiaoZhi hardware configurations**

## 🎯 Giới Thiệu

MeiLin ESP32 Firmware là firmware chính thức cho hệ thống MeiLin AI, được thiết kế để tương thích hoàn toàn với tất cả các board ESP32 mà XiaoZhi hỗ trợ. Với 80+ board configurations và pin mappings giống hệt XiaoZhi, người dùng có thể dễ dàng chuyển đổi từ XiaoZhi sang MeiLin mà không cần thay đổi phần cứng.

## ✨ Tính Năng

### ✅ Tương Thích Hoàn Toàn
- **80+ ESP32 Boards** - Giống hệt XiaoZhi
- **Pin Configurations** - Identical pin mappings
- **Hardware Abstraction** - Tương thích với tất cả peripherals
- **Display Support** - LCD, OLED, ePaper
- **Audio Codecs** - Multiple audio format support

### 🎤 Voice AI Integration
- **Wake Word Detection** - "MeiLin" wake word
- **Real-time Audio** - Audio input/output processing
- **TTS Integration** - Text-to-speech với ElevenLabs
- **Voice Commands** - Natural language processing

### 🌐 Network Communication
- **HTTP Client** - Communication với MeiLin backend
- **WebSocket Support** - Real-time bidirectional communication
- **MQTT Protocol** - IoT messaging
- **WiFi Management** - Auto-connect và reconnection

### 🎨 Hardware Control
- **LED Feedback** - RGB LED status indicators
- **Display Interface** - Text và graphic display
- **Camera Support** - ESP32 camera modules
- **Battery Monitoring** - Power management

## 📋 Danh Sách Board Hỗ Trợ

### 🏷️ Popular Boards
- **ESP-BOX** - Official ESP32 development board
- **M5Stack Series** - M5Stack Core S3, Tab5
- **LilyGO Series** - T-CameraPlus, T-Display
- **Waveshare Series** - Multiple display boards
- **ATK Series** - DNESP32S3 variants
- **XingZhi Cube** - Multiple configurations

### 🔧 Board Categories
- **WiFi Only** - Standard WiFi boards
- **4G/LTE** - Boards với cellular connectivity
- **Display** - Boards với integrated displays
- **Camera** - Boards với camera modules
- **Audio** - Boards với audio codecs

## 🚀 Cài Đặt

### Prerequisites
- **ESP-IDF** v5.1+
- **Python** 3.8+
- **CMake** 3.16+

### Build Instructions
```bash
# Clone repository
git clone https://github.com/truongcongdinh97/PROJECT_MEILIN_AIVTUBER.git
cd PROJECT_MEILIN_AIVTUBER/meilin-esp32

# Configure project
idf.py set-target esp32s3  # or your target board
idf.py menuconfig

# Build và flash
idf.py build
idf.py -p COM3 flash monitor  # replace COM3 với port của bạn
```

### Configuration
1. **WiFi Settings** - Cấu hình SSID và password
2. **Backend URL** - URL của MeiLin backend server
3. **Device ID** - Unique identifier cho device
4. **Audio Settings** - Audio codec và sample rate

## 🔧 Hardware Setup

### Pin Mappings
Tất cả pin mappings đều giống hệt XiaoZhi:

| Component | Pin | Description |
|-----------|-----|-------------|
| I2S Data | GPIO35 | Audio input data |
| I2S Clock | GPIO36 | Audio clock |
| I2S WS | GPIO37 | Word select |
| LED Data | GPIO38 | RGB LED data |
| Display SDA | GPIO39 | I2C data |
| Display SCL | GPIO40 | I2C clock |

### Power Management
- **Battery Monitoring** - Real-time battery level
- **Power Saving** - Auto sleep/wake
- **Charging Detection** - USB/DC charging status

## 📡 API Integration

### Backend Communication
```cpp
// Wake word detection
meilin_client.SendWakeEvent(0.95f);

// Chat message
auto response = meilin_client.SendChatMessage("Xin chào MeiLin");

// Voice command
auto result = meilin_client.SendCommand("bật đèn phòng khách");

// Audio download
auto audio_data = meilin_client.DownloadAudio(audio_url);
```

### Supported Endpoints
- `POST /api/wake` - Wake word detection
- `POST /api/chat` - Chat communication
- `POST /api/command` - Voice commands
- `GET /api/audio/{id}` - Audio download
- `GET /health` - Health check

## 🎛️ Configuration

### Board Selection
Trong `menuconfig`:
```
Component config → MeiLin Configuration → Board Selection
```

### Network Settings
```cpp
// WiFi configuration
app.ConnectToWiFi("YourSSID", "YourPassword");

// Backend configuration
meilin_client.SetBackendUrl("https://your-meilin-backend.com");
```

## 🔄 OTA Updates

MeiLin ESP32 hỗ trợ Over-the-Air updates:

```bash
# Build OTA image
idf.py build

# Upload OTA
idf.py -p COM3 flash
```

## 🐛 Debugging

### Log Levels
```cpp
// Set log level
esp_log_level_set("*", ESP_LOG_INFO);
esp_log_level_set("MeiLinClient", ESP_LOG_DEBUG);
```

### Common Issues
1. **WiFi Connection** - Kiểm tra SSID/password
2. **Audio Issues** - Kiểm tra codec configuration
3. **Display Problems** - Verify pin mappings
4. **Backend Communication** - Check network connectivity

## 📊 Performance

### Memory Usage
- **Free Heap**: ~150KB
- **PSRAM Usage**: ~2MB (nếu có)
- **Flash Usage**: ~3MB

### Power Consumption
- **Active**: ~120mA
- **Sleep**: ~10mA
- **Deep Sleep**: ~5μA

## 🤝 Contributing

Chúng tôi hoan nghênh mọi đóng góp! Vui lòng:

1. Fork repository
2. Tạo feature branch
3. Commit changes
4. Push to branch
5. Tạo Pull Request

## 📄 License

MeiLin ESP32 Firmware được phát hành dưới MIT License.

## 🔗 Links

- **GitHub Repository**: https://github.com/truongcongdinh97/PROJECT_MEILIN_AIVTUBER
- **Documentation**: https://github.com/truongcongdinh97/PROJECT_MEILIN_AIVTUBER/docs
- **Issues**: https://github.com/truongcongdinh97/PROJECT_MEILIN_AIVTUBER/issues

## 🆘 Support

Nếu bạn gặp vấn đề:
1. Kiểm tra [Issues](https://github.com/truongcongdinh97/PROJECT_MEILIN_AIVTUBER/issues)
2. Tạo new issue với detailed description
3. Join community discussion

---

**MeiLin ESP32 Firmware - Tương thích hoàn toàn với XiaoZhi hardware!** 🚀
