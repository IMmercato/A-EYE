#include "camera_manager.h"
#include "config.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_psram.h"

static const char *TAG = "CAMERA_MANAGER";

esp_err_t camera_init(void) {
    ESP_LOGI(TAG, "Starting camera initialization...");
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sccb_sda = CAMERA_PIN_SIOD;
    config.pin_sccb_scl = CAMERA_PIN_SIOC;
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = CAMERA_FRAME_SIZE;
    config.jpeg_quality = CAMERA_JPEG_QUALITY;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;

    // Log della configurazione
    ESP_LOGI(TAG, "Camera config: XCLK=%d, SIOD=%d, SIOC=%d", 
             config.pin_xclk, config.pin_sccb_sda, config.pin_sccb_scl);
    ESP_LOGI(TAG, "Frame size: %d, JPEG quality: %d", 
             config.frame_size, config.jpeg_quality);

    if (!esp_psram_is_initialized()) {
        ESP_LOGE(TAG, "PSRAM not initialized! Camera may fail.");
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x (%s)", err, esp_err_to_name(err));
        return err;
    }

    // Test di acquisizione immediata
    ESP_LOGI(TAG, "Testing camera capture...");
    camera_fb_t* test_fb = esp_camera_fb_get();
    if (test_fb) {
        ESP_LOGI(TAG, "Test capture successful! Size: %d bytes", test_fb->len);
        esp_camera_fb_return(test_fb);
    } else {
        ESP_LOGE(TAG, "Test capture failed!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}

camera_fb_t* camera_capture_frame(void) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return NULL;
    }
    return fb;
}

void camera_return_frame(camera_fb_t* fb) {
    esp_camera_fb_return(fb);
}