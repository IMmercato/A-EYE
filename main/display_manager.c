#include "display_manager.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"   // Display Libraries
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

static const char *TAG = "DISPLAY_MANAGER";
static esp_lcd_panel_handle_t panel = NULL;

esp_err_t display_init(void) {
    // Initialize GPIO for LED
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

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_CS_GPIO,
        .dc_gpio_num = LCD_DC_GPIO,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .dc_low_on_data = 0,
        },
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };
    
    esp_err_t ret = esp_lcd_new_panel_io_spi(&io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_GPIO,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
        .vendor_config = NULL,
    };

    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel);
    esp_lcd_panel_reset(panel);
    esp_lcd_panel_init(panel);
    esp_lcd_panel_disp_on_off(panel, true);

    ESP_LOGI(TAG, "LCD display initialized");
    return ESP_OK;
}

void display_show_text(const char* text) {
    ESP_LOGI(TAG, "DISPLAY: %s", text);

    // Display the text on the display
    esp_lcd_panel_draw_bitmap(panel, 0, 0, 240, 240, NULL);
}

void display_blink_status(void) {
    gpio_set_level(LED_GPIO_NUM, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(LED_GPIO_NUM, 1);
}