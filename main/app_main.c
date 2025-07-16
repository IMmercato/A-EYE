#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_httpd.h"

static const char *TAG = "app_main";

// --- SPIFFS Initialization ---
void init_spiffs(void) {
    ESP_LOGI(TAG, "Mounting SPIFFS...");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS size: %d, used: %d", total, used);
    }
}

// --- Wi-Fi Setup (Stub - should implement full init) ---
void start_wifi(void) {
    // Initialize networking
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Galaxy A53 5G61C5",
            .password = "xfuw1104"
        }
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Wi-Fi started. Connecting to %s...", wifi_config.sta.ssid);
}

// --- Main App Entry ---
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    start_wifi();
    init_spiffs(); // Only needed if serving files from SPIFFS
    start_webserver(); // Starts your HTTP server (in app_httpd.c)

    ESP_LOGI(TAG, "Webserver running");
}