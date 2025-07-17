#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "wifi_manager.h"
#include "camera_manager.h"
#include "server_comm.h"
#include "display_manager.h"
#include "ai_processor.h"

static const char *TAG = "A-EYE";

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "A_EYE Starting...");

    // Initialize modules
    display_init();
    display_show_text("Starting...");

    wifi_init();
    camera_init();
    server_comm_init();
    ai_processor_init();

    display_show_text("Ready!");

    // Start main processing task
    xTaskCreate(ai_processing_task, "ai_task", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "A-EYE Initialized Successfully");
}