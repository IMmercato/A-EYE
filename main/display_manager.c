#include "display_manager.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"          // Required for SPI bus setup
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"           // Display Libraries
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
    ESP_LOGI(TAG, "LED initialized");

    // Configure SPI bus
    spi_bus_config_t bus_config = {
        .sclk_io_num = LCD_CLK_GPIO,
        .mosi_io_num = LCD_MOSI_GPIO,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 240 * 2, // RGB565
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure panel IO
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC_GPIO,
        .cs_gpio_num = LCD_CS_GPIO,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };

    esp_lcd_panel_io_handle_t io_handle = NULL;
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        return ret;
    }

    // Panel Configuration
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_GPIO,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
        .vendor_config = NULL,
    };

    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel driver: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_reset(panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel reset failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_init(panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_disp_on_off(panel, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn display on: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "LCD display initialized successfully");
    return ESP_OK;
}

void display_show_text(const char* text) {
    ESP_LOGI(TAG, "DISPLAY: %s", text);

    // Draw a solid background (for now)
    uint16_t color = 0x0000; // Black
    esp_lcd_panel_draw_bitmap(panel, 0, 0, 240, 240, &color);

    // 🚀 Text rendering will need LVGL or other font/image renderer
    // Let me know if you'd like to add that next
}

void display_blink_status(void) {
    gpio_set_level(LED_GPIO_NUM, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LED_GPIO_NUM, 1);
}