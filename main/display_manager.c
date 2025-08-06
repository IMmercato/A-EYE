#include "display_manager.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "font5x7.h"
#include <string.h>

#define font_5x7 Font5x7

static const char *TAG = "DISPLAY_MANAGER";
static esp_lcd_panel_handle_t panel = NULL;

esp_err_t display_init(void) {
    // Initialize LED GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO_NUM),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    gpio_set_level(LED_GPIO_NUM, 1);
    ESP_LOGI(TAG, "LED initialized");

    // Backlight GPIO configuration
    gpio_config_t bl_conf = {
        .pin_bit_mask = 1ULL << LCD_BL_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&bl_conf);
    gpio_set_level(LCD_BL_GPIO, 0); // 1
    ESP_LOGI(TAG, "Backlight initialized");

    // Configure SPI bus
    spi_bus_config_t bus_config = {
        .sclk_io_num = LCD_CLK_GPIO,
        .mosi_io_num = LCD_MOSI_GPIO,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 240 * 2 + 8,
    };
    ESP_LOGI(TAG, "Initializing SPI bus...");
    esp_err_t ret = spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed SPI init: %s", esp_err_to_name(ret));
        return ret;
    }

    // Panel IO configuration
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC_GPIO,
        .cs_gpio_num = LCD_CS_GPIO,
        .pclk_hz = 20 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_LOGI(TAG, "Creating panel IO...");
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &io_handle);
    if (ret != ESP_OK || io_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        spi_bus_free(LCD_SPI_HOST);
        return ret;
    }

    // Panel driver configuration
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_GPIO,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_LOGI(TAG, "Creating ST7789 panel...");
    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel: %s", esp_err_to_name(ret));
        spi_bus_free(LCD_SPI_HOST);
        return ret;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, false)); // true
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

    uint16_t black = 0x0000;
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, 0, 240, 240, &black));

    ESP_LOGI(TAG, "LCD display initialized successfully");
    return ESP_OK;
}

void display_show_text(const char* text) {
    if (!panel || !text) return;
    ESP_LOGI(TAG, "DISPLAY: %s", text);

    uint16_t bg_color = 0x0000;
    esp_lcd_panel_draw_bitmap(panel, 0, 0, 240, 240, &bg_color);

    uint16_t fg_color = 0xFFFF;
    int x_pos = 10;
    int y_pos = 50;

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c < 32 || c > 127) continue;
        int font_index = (c - 32) * 5;
        const char* char_data = &font_5x7[font_index];

        int char_x = x_pos + i * 6;
        for (int y = 0; y < 7; y++) {
            for (int x = 0; x < 5; x++) {
                if (char_data[x] & (1 << y)) {
                    esp_lcd_panel_draw_bitmap(panel,
                        char_x + x, y_pos + y,
                        char_x + x + 1, y_pos + y + 1,
                        &fg_color);
                }
            }
        }
    }
}

void display_test_pattern(void) {
    if (!panel) return;
    uint16_t colors[] = { 0xF800, 0x07E0, 0x001F, 0xFFFF, 0x0000 };
    for (int i = 0; i < 5; i++) {
        esp_lcd_panel_draw_bitmap(panel, 0, 0, 240, 240, &colors[i]);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void display_blink_status(void) {
    gpio_set_level(LED_GPIO_NUM, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LED_GPIO_NUM, 1);
}