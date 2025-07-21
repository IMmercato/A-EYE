#include "camera_manager.h"
#include "config.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_psram.h"

static const char *TAG = "CAMERA_MANAGER";

esp_err_t camera_init(void) {
    ESP_LOGI(TAG, "Starting camera initialization...");

    // Ensure PSRAM is initialized
    if (!esp_psram_is_initialized()) {
        ESP_LOGE(TAG, "PSRAM not initialized!");
        return ESP_ERR_INVALID_STATE;
    }

    camera_config_t config = {
        .ledc_channel = LEDC_CHANNEL_0,
        .ledc_timer = LEDC_TIMER_0,
        .pin_d0 = CAMERA_PIN_D0,
        .pin_d1 = CAMERA_PIN_D1,
        .pin_d2 = CAMERA_PIN_D2,
        .pin_d3 = CAMERA_PIN_D3,
        .pin_d4 = CAMERA_PIN_D4,
        .pin_d5 = CAMERA_PIN_D5,
        .pin_d6 = CAMERA_PIN_D6,
        .pin_d7 = CAMERA_PIN_D7,
        .pin_xclk = CAMERA_PIN_XCLK,
        .pin_pclk = CAMERA_PIN_PCLK,
        .pin_vsync = CAMERA_PIN_VSYNC,
        .pin_href = CAMERA_PIN_HREF,
        .pin_sccb_sda = CAMERA_PIN_SIOD,
        .pin_sccb_scl = CAMERA_PIN_SIOC,
        .pin_pwdn = CAMERA_PIN_PWDN,
        .pin_reset = CAMERA_PIN_RESET,
        .xclk_freq_hz = 10000000,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = CAMERA_FRAME_SIZE,
        .jpeg_quality = CAMERA_JPEG_QUALITY,
        .fb_count = 2,
        .fb_location = CAMERA_FB_IN_PSRAM, // Use PSRAM
    };

    ESP_LOGI(TAG, "Initializing camera with PSRAM support");

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x (%s)", err, esp_err_to_name(err));
        return err;
    }

    // Test capture
    camera_fb_t* test_fb = esp_camera_fb_get();
    if (!test_fb) {
        ESP_LOGE(TAG, "Camera test capture failed!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Test capture successful! Size: %d bytes", test_fb->len);
    esp_camera_fb_return(test_fb);

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
    if (fb) {
        esp_camera_fb_return(fb);
    }
}