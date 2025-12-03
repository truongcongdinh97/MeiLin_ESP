# MeiLin ESP32 Firmware

**Firmware tương thích hoàn toàn với XiaoZhi hardware configurations**

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.3-blue)](https://docs.espressif.com/projects/esp-idf/)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

## 🎯 Giới Thiệu

MeiLin ESP32 Firmware là firmware AI Voice Assistant, được thiết kế để tương thích hoàn toàn với **XiaoZhi hardware** (80+ boards). 

### 🌟 Hai Chế Độ Sử Dụng

| Mode | Backend | Chi phí | Tính năng |
|------|---------|---------|-----------|
| **XiaoZhi Mode** | XiaoZhi Cloud | **MIỄN PHÍ** | AI chat, TTS, Wake word |
| **MeiLin Mode** | Self-hosted | Tùy API | Custom persona, Multi-user, Telegram |

> **💡 Mặc định firmware kết nối XiaoZhi Cloud - KHÔNG cần server riêng!**

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

## 🚀 Quick Start (XiaoZhi Mode - MIỄN PHÍ)

### Cách 1: Tải firmware sẵn (Nhanh nhất)
1. Tải firmware từ [Releases](https://github.com/YOUR_GITHUB_USERNAME/MeiLin_ESP/releases)
2. Flash bằng [ESP Web Flasher](https://web.esphome.io/) hoặc `esptool`
3. Kết nối WiFi qua Bluetooth/SmartConfig
4. **Xong!** Nói "Hi Lexin" để bắt đầu chat

### Cách 2: Build từ source
```bash
# Clone repository
git clone https://github.com/YOUR_GITHUB_USERNAME/MeiLin_ESP.git
cd MeiLin_ESP

# Chọn board
idf.py set-target esp32s3
idf.py menuconfig
# → Xiaozhi Assistant → Board Type → Chọn board của bạn
# → Xiaozhi Assistant → Default Language → Vietnamese

# Build và flash
idf.py build
idf.py -p COM3 flash monitor
```

### Sau khi flash:
1. **Kết nối WiFi**: Dùng SmartConfig hoặc web portal
2. **Đăng ký XiaoZhi**: Truy cập xiaozhi.me để đăng ký device
3. **Sử dụng**: Nói **"Hi Lexin"** → Đặt câu hỏi → Nhận trả lời!

> 💡 **Mặc định dùng XiaoZhi Cloud (miễn phí)** - Không cần setup server!

---

## 🔧 Nâng cao: MeiLin Mode (Self-hosted)

Muốn custom persona, multi-user, Telegram bot? Chuyển sang MeiLin backend:

### Bước 1: Deploy MeiLin Server
```bash
git clone https://github.com/YOUR_GITHUB_USERNAME/MeiLin_Server.git
cd MeiLin_Server
cp .env.example .env
# Điền API keys vào .env
docker-compose up -d
```

### Bước 2: Cấu hình ESP32
```bash
idf.py menuconfig
# → Xiaozhi Assistant → OTA URL → https://your-domain.com/api/ota/
```

### Bước 3: Flash
```bash
idf.py build flash
```

---

## 📋 Prerequisites
- **ESP-IDF** v5.3+
- **Python** 3.8+
- **CMake** 3.16+

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

- **GitHub Repository**: https://github.com/YOUR_GITHUB_USERNAME/MeiLin_ESP
- **Documentation**: https://github.com/YOUR_GITHUB_USERNAME/MeiLin_ESP/docs
- **Issues**: https://github.com/YOUR_GITHUB_USERNAME/MeiLin_ESP/issues

## 🆘 Support

Nếu bạn gặp vấn đề:
1. Kiểm tra [Issues](https://github.com/YOUR_GITHUB_USERNAME/MeiLin_ESP/issues)
2. Tạo new issue với detailed description
3. Join community discussion

---

**MeiLin ESP32 Firmware - Tương thích hoàn toàn với XiaoZhi hardware!** 🚀
