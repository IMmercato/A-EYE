#include "esp_camera.h"
#include "esp_log.h"

void init_camera(void) {
    camera_config_t config = {
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = 15,
        .pin_sccb_sda = 8,
        .pin_sccb_scl = 7,
        .pin_d7 = 38,
        .pin_d6 = 39,
        .pin_d5 = 40,
        .pin_d4 = 41,
        .pin_d3 = 42,
        .pin_d2 = 12,
        .pin_d1 = 11,
        .pin_d0 = 10,
        .pin_vsync = 6,
        .pin_href = 5,
        .pin_pclk = 4,
        .xclk_freq_hz = 20000000,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_QVGA,
        .jpeg_quality = 12,
        .fb_count = 2
    };
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE("CAMERA", "Camera initialization failed with error: %s", esp_err_to_name(err));
        return;
    }
}