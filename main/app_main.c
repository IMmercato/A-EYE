#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "lvgl.h"

#include "wifi_manager.h"
#include "camera_manager.h"
#include "server_comm.h"
#include "display_manager.h"
#include "ai_processor.h"

static const char *TAG = "A-EYE";

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "A_EYE Starting...");

// Display Initialize
#ifdef DISPLAY_RESET_PIN
    if (GPIO_IS_VALID_GPIO(DISPLAY_RESET_PIN))
    {
        gpio_reset_pin(DISPLAY_RESET_PIN);
        gpio_set_direction(DISPLAY_RESET_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(DISPLAY_RESET_PIN, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(DISPLAY_RESET_PIN, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
#endif

    // Initialize LVGL
    lv_init();

    // Initialize modules
    display_init();
    display_test_pattern();
    display_show_text("Starting...");

    wifi_init();
    display_show_text("WIFI Connecting...");

    wifi_wait_for_connection();
    ESP_LOGI(TAG, "WiFi Connected!");
    display_show_text("WiFi Connected!");
    camera_init();
    server_comm_init();
    ai_processor_init();

    display_show_text("Ready!");

    // Start main processing task
    xTaskCreate(ai_processing_task, "ai_task", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "A-EYE Initialized Successfully");
}