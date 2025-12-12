#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// =============================================================================
// ESP32-S3-WROOM-1-N16R8 Board Configuration (XiaoZhi Standard)
// Hardware: ESP32-S3 Module + INMP441 + MAX98357A + GC9A01
// 
// This is a generic ESP32-S3 module with:
// - 16MB Flash (N16)
// - 8MB PSRAM (R8)
// - WiFi + Bluetooth 5.0
// =============================================================================

// Audio Configuration
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// 如果使用 Duplex I2S 模式，请注释下面一行
#define AUDIO_I2S_METHOD_SIMPLEX

#ifdef AUDIO_I2S_METHOD_SIMPLEX

// =============================================================================
// INMP441 Microphone I2S Pins (XiaoZhi Standard)
// =============================================================================
// INMP441 Pin    ESP32-S3 GPIO    Description
// VDD            3V3              Power 3.3V
// GND            GND              Ground
// SD             GPIO6            Data output from mic
// WS             GPIO4            Word Select (LRCK)
// SCK            GPIO5            Bit Clock
// L/R            GND              Left channel select

#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_4
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_5
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_6

// =============================================================================
// MAX98357A Speaker I2S Pins (XiaoZhi Standard)
// =============================================================================
// MAX98357A Pin  ESP32-S3 GPIO    Description
// VIN            3V3 or 5V        Power
// GND            GND              Ground
// DIN            GPIO7            Data input to speaker
// BCLK           GPIO15           Bit Clock
// LRC            GPIO16           Left/Right Clock
// SD             GPIO18           Shutdown (HIGH=on, LOW=off)
// GAIN           NC or GND        Gain control (optional)

#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_7
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_15
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_16
#define AUDIO_CODEC_PA_PIN      GPIO_NUM_18   // MAX98357A SD (Shutdown/Enable)

#else

// Duplex fallback (shared clock)
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_4
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_5
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_6
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_7

#endif

// =============================================================================
// Button Configuration
// =============================================================================
#define BUILTIN_LED_GPIO        GPIO_NUM_48
#define BOOT_BUTTON_GPIO        GPIO_NUM_0
#define TOUCH_BUTTON_GPIO       GPIO_NUM_NC
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_NC

// =============================================================================
// Display SPI Pins (XiaoZhi Standard for GC9A01)
// =============================================================================
// GC9A01 Pin     ESP32-S3 GPIO    Description
// VCC            3V3              Power 3.3V
// GND            GND              Ground
// SCL            GPIO21           SPI Clock
// SDA            GPIO47           SPI Data (MOSI)
// CS             GPIO41           Chip Select
// DC             GPIO40           Data/Command
// RST            GPIO45           Reset
// BL             GPIO42           Backlight

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_42
#define DISPLAY_MOSI_PIN      GPIO_NUM_47
#define DISPLAY_CLK_PIN       GPIO_NUM_21
#define DISPLAY_DC_PIN        GPIO_NUM_40
#define DISPLAY_RST_PIN       GPIO_NUM_45
#define DISPLAY_CS_PIN        GPIO_NUM_41

// =============================================================================
// Display Configuration - GC9A01 240x240 (Default)
// =============================================================================
#ifdef CONFIG_LCD_GC9A01_240X240
#define LCD_TYPE_GC9A01_SERIAL
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_INVERT_COLOR    true
#define DISPLAY_RGB_ORDER  LCD_RGB_ELEMENT_ORDER_BGR
#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 0
#endif

// =============================================================================
// Display Configuration - ST7789 240x320
// =============================================================================
#ifdef CONFIG_LCD_ST7789_240X320
#define LCD_TYPE_ST7789_SERIAL
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  320
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_INVERT_COLOR    true
#define DISPLAY_RGB_ORDER  LCD_RGB_ELEMENT_ORDER_RGB
#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 0
#endif

// =============================================================================
// Default Display (if no CONFIG selected) - GC9A01 240x240
// =============================================================================
#if !defined(LCD_TYPE_GC9A01_SERIAL) && !defined(LCD_TYPE_ST7789_SERIAL)
#define LCD_TYPE_GC9A01_SERIAL
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_INVERT_COLOR    true
#define DISPLAY_RGB_ORDER  LCD_RGB_ELEMENT_ORDER_BGR
#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 0
#endif

// =============================================================================
// MCP Test: Control a lamp (optional)
// =============================================================================
#define LAMP_GPIO GPIO_NUM_NC  // Disabled - GPIO18 used for speaker SD

#endif // _BOARD_CONFIG_H_
