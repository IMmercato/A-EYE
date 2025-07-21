#ifndef CONFIG_H
#define CONFIG_H

// Wifi Configuration
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Server Configuration
#define SERVER_URL "https://a-eye-n8jr.onrender.com/analyze"
#define SERVER_TIMEOUT_MS 30000

// Camera Configuration
#define CAMERA_FRAME_SIZE FRAMESIZE_QQVGA
#define CAMERA_JPEG_QUALITY 15
#define CAPTURE_INTERVAL_MS 3000

// GPIO Configurattion
#define LED_GPIO_NUM 3

// LCD panel Configuration
#define LCD_MOSI_GPIO   11
#define LCD_CLK_GPIO    12
#define LCD_CS_GPIO     10
#define LCD_DC_GPIO     9
#define LCD_RST_GPIO    8
#define LCD_GPIO_NUM    21
//#define LCD_SPI_HOST    SPI2_HOST

// Hardware Pins(for ESP32-S3-EYE)
#define CAMERA_PIN_PWDN    -1
#define CAMERA_PIN_RESET   -1
#define CAMERA_PIN_XCLK    15
#define CAMERA_PIN_SIOD    4
#define CAMERA_PIN_SIOC    5
#define CAMERA_PIN_D0      11
#define CAMERA_PIN_D1      9
#define CAMERA_PIN_D2      8
#define CAMERA_PIN_D3      10
#define CAMERA_PIN_D4      12
#define CAMERA_PIN_D5      18
#define CAMERA_PIN_D6      17
#define CAMERA_PIN_D7      16
#define CAMERA_PIN_VSYNC   6
#define CAMERA_PIN_HREF    7
#define CAMERA_PIN_PCLK    13

#endif