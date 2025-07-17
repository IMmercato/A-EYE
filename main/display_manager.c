#include "display_manager.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DISPLAY_MANAGER";

esp_err_t display_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO_NUM),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_set_level(LED_GPIO_NUM, 1);
    ESP_LOGI(TAG, "Display initialized");
    return ESP_OK;
}

void display_show_text(const char* text) {
    ESP_LOGI(TAG, "DISPLAY: %s", text);
}

void display_blink_status(void) {
    gpio_set_level(LED_GPIO_NUM, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(LED_GPIO_NUM, 1);
}