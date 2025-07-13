#include "esp_log.h"
#include "camera_init.h"
#include "web_server.h"
#include "face_logic.h"

void app_main(void) {
    ESP_LOGI("MAIN", "Starting application...");

    init_camera();
    init_face_logic();
    start_web_server();
}