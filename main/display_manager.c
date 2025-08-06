#include "display_manager.h"
#include "config.h"
#include "bsp/esp32_s3_eye.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include <string.h>

static const char* TAG = "DISPLAY_MANAGER";
static lv_obj_t* camera_canvas = NULL;
static uint8_t* cam_buff = NULL;
static size_t cam_buff_size = 0;

void display_init(void)
{
    // Init LED GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO_NUM,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO_NUM, 1);

    // Init display
    bsp_display_start();
    bsp_display_backlight_on();

    ESP_LOGI(TAG, "Display initialized");
}

void display_show_text(const char* text)
{
    if (!text) return;

    lv_obj_clean(lv_scr_act()); // Clear current screen
    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    ESP_LOGI(TAG, "Displayed text: %s", text);
}

void display_test_pattern(void)
{
    lv_obj_clean(lv_scr_act());
    lv_color_t colors[] = {
        lv_color_hex(0xFF0000), // Red
        lv_color_hex(0x00FF00), // Green
        lv_color_hex(0x0000FF), // Blue
        lv_color_hex(0xFFFFFF), // White
        lv_color_hex(0x000000)  // Black
    };

    for (int i = 0; i < 5; i++) {
        lv_obj_set_style_bg_color(lv_scr_act(), colors[i], 0);
        lv_obj_invalidate(lv_scr_act());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void display_blink_status(void)
{
    gpio_set_level(LED_GPIO_NUM, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LED_GPIO_NUM, 1);
}

void display_prepare_camera_canvas(void)
{
    // Allocate buffer only once
    if (!cam_buff) {
        cam_buff_size = BSP_LCD_H_RES * BSP_LCD_V_RES * 2;
        cam_buff = heap_caps_malloc(cam_buff_size, MALLOC_CAP_SPIRAM);
        assert(cam_buff);
    }

    // Create canvas only once
    if (!camera_canvas) {
        camera_canvas = lv_canvas_create(lv_scr_act());
        lv_canvas_set_buffer(camera_canvas, cam_buff, BSP_LCD_H_RES, BSP_LCD_V_RES, LV_IMG_CF_TRUE_COLOR);
        lv_obj_center(camera_canvas);
    }
}

void display_show_camera_frame(camera_fb_t* frame)
{
    if (!frame || !camera_canvas || !cam_buff) return;

    memcpy(cam_buff, frame->buf, cam_buff_size);

#if BSP_LCD_BIGENDIAN
    lv_draw_sw_rgb565_swap(cam_buff, cam_buff_size);
#endif

    lv_obj_invalidate(camera_canvas);
}