#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// =============================================================================
// MeiLin ESP32-CAM Board Configuration  
// Hardware: ESP32-CAM + INMP441 + MAX98357A (Audio only first!)
// 
// ESP32-CAM GPIO ANALYSIS:
// ========================
// CAMERA USES: 0,5,18,19,21,22,23,25,26,27,32,34,35,36,39 (15 pins!)
// SERIAL:      1(TX), 3(RX) - used for programming/debug
// BOOT:        0 (shared with camera XCLK, LOW=boot mode)
// 
// ACTUALLY FREE PINS on ESP32-CAM:
// - GPIO2:  OK but must be LOW/floating for boot
// - GPIO4:  Camera Flash LED (can repurpose)  
// - GPIO12: HSPI MISO (must be LOW for boot)
// - GPIO13: HSPI MOSI
// - GPIO14: HSPI CLK
// - GPIO15: HSPI CS (must be HIGH for boot, or no serial output)
// - GPIO16: U2RXD (used by PSRAM on WROVER - check your board!)
// 
// CONCLUSION: Only 6 usable pins, need 6 for audio, 0 left for display!
// =============================================================================

// Audio Configuration
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// Use Simplex I2S (separate mic and speaker channels)
#define AUDIO_I2S_METHOD_SIMPLEX

#ifdef AUDIO_I2S_METHOD_SIMPLEX

// INMP441 Microphone I2S Pins
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_15   // INMP441 WS (LRC)
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_14   // INMP441 SCK (BCLK)
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_2    // INMP441 SD (data out)

// MAX98357A Speaker I2S Pins  
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_12   // MAX98357A DIN
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_13   // MAX98357A BCLK
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_4    // MAX98357A LRC

#else
// Duplex fallback (not used)
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_15
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_14
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_2
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_12
#endif

// =============================================================================
// Button Configuration
// =============================================================================
#define BOOT_BUTTON_GPIO        GPIO_NUM_0    // Built-in BOOT button (shared with camera)

// =============================================================================
// LED Configuration  
// =============================================================================
#define BUILTIN_LED_GPIO        GPIO_NUM_33   // Built-in red LED (active LOW)
// #define FLASH_LED_GPIO       GPIO_NUM_4    // Camera flash LED (we use for audio)

// =============================================================================
// Display Configuration - DISABLED (no free GPIO!)
// =============================================================================
// #define LCD_TYPE_GC9A01_SERIAL

// All display pins set to NC (Not Connected)
#define DISPLAY_MOSI_PIN        GPIO_NUM_NC
#define DISPLAY_SCLK_PIN        GPIO_NUM_NC
#define DISPLAY_CS_PIN          GPIO_NUM_NC
#define DISPLAY_DC_PIN          GPIO_NUM_NC
#define DISPLAY_RESET_PIN       GPIO_NUM_NC
#define DISPLAY_BACKLIGHT_PIN   GPIO_NUM_NC

#define DISPLAY_SPI_SCLK_HZ     (40 * 1000 * 1000)
#define DISPLAY_SPI_MODE        0

#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          240
#define DISPLAY_MIRROR_X        false
#define DISPLAY_MIRROR_Y        false
#define DISPLAY_SWAP_XY         false
#define DISPLAY_INVERT_COLOR    true
#define DISPLAY_RGB_ORDER       LCD_RGB_ELEMENT_ORDER_BGR
#define DISPLAY_OFFSET_X        0
#define DISPLAY_OFFSET_Y        0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

// =============================================================================
// Camera Configuration - DISABLED for now
// Enable later when we implement camera-server streaming
// =============================================================================
// #define ENABLE_CAMERA

#ifdef ENABLE_CAMERA
// AI-Thinker ESP32-CAM OV2640 pinout
#define CAMERA_PIN_PWDN     GPIO_NUM_32
#define CAMERA_PIN_RESET    GPIO_NUM_NC
#define CAMERA_PIN_XCLK     GPIO_NUM_0
#define CAMERA_PIN_SIOD     GPIO_NUM_26  // I2C SDA
#define CAMERA_PIN_SIOC     GPIO_NUM_27  // I2C SCL

#define CAMERA_PIN_D7       GPIO_NUM_35
#define CAMERA_PIN_D6       GPIO_NUM_34
#define CAMERA_PIN_D5       GPIO_NUM_39
#define CAMERA_PIN_D4       GPIO_NUM_36
#define CAMERA_PIN_D3       GPIO_NUM_21
#define CAMERA_PIN_D2       GPIO_NUM_19
#define CAMERA_PIN_D1       GPIO_NUM_18
#define CAMERA_PIN_D0       GPIO_NUM_5

#define CAMERA_PIN_VSYNC    GPIO_NUM_25
#define CAMERA_PIN_HREF     GPIO_NUM_23
#define CAMERA_PIN_PCLK     GPIO_NUM_22

#define XCLK_FREQ_HZ        20000000
#endif

#endif // _BOARD_CONFIG_H_
