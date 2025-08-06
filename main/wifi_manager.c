#include "wifi_manager.h"
#include "config.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "driver/gpio.h"
#include "freertos/task.h"

static const char *TAG = "WIFI_MANAGER";
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static int retry_count = 0;
static const int MAX_RETRIES = 10;
static int full_retry_count = 0;
static const int MAX_FULL_RETRIES = 3;

static bool wifi_connected = false;
static bool wifi_failed = false;

static void log_disconnect_reason(int reason) {
    switch (reason) {
        case WIFI_REASON_AUTH_EXPIRE: ESP_LOGW(TAG, "AUTH_EXPIRE"); break;
        case WIFI_REASON_AUTH_FAIL: ESP_LOGW(TAG, "AUTH_FAIL"); break;
        case WIFI_REASON_NO_AP_FOUND: ESP_LOGW(TAG, "NO_AP_FOUND"); break;
        case WIFI_REASON_ASSOC_LEAVE: ESP_LOGW(TAG, "ASSOCIATION_LEAVE"); break;
        case WIFI_REASON_CONNECTION_FAIL: ESP_LOGW(TAG, "CONNECTION_FAIL"); break;
        case WIFI_REASON_BEACON_TIMEOUT: ESP_LOGW(TAG, "BEACON_TIMEOUT"); break;
        default: ESP_LOGW(TAG, "Unknown reason: %d", reason); break;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Wi-Fi started → attempting connection...");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* disconn = (wifi_event_sta_disconnected_t*) event_data;
                ESP_LOGW(TAG, "Disconnected from Wi-Fi");
                log_disconnect_reason(disconn->reason);

                if (retry_count < MAX_RETRIES) {
                    retry_count++;
                    ESP_LOGI(TAG, "Retrying (%d/%d)...", retry_count, MAX_RETRIES);
                    esp_wifi_connect();
                } else {
                    full_retry_count++;
                    ESP_LOGE(TAG, "Max retries reached. Restarting Wi-Fi (%d/%d)...", full_retry_count, MAX_FULL_RETRIES);
                    esp_wifi_stop();
                    esp_wifi_start();
                    retry_count = 0;

                    if (full_retry_count >= MAX_FULL_RETRIES) {
                        ESP_LOGE(TAG, "Exceeded full retry limit. Restarting device...");
                        esp_restart();
                    }
                }
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            }

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to Wi-Fi.");
                wifi_connected = true;
                wifi_failed = false;
                break;
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        retry_count = 0;
        full_retry_count = 0;
    }
}

static void led_wifi_indicator_task(void *pvParameter) {
    gpio_reset_pin(LED_GPIO_NUM);
    gpio_set_direction(LED_GPIO_NUM, GPIO_MODE_OUTPUT);

    while (1) {
        if (wifi_connected) {
            gpio_set_level(LED_GPIO_NUM, 1);  // Solid ON
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else if (wifi_failed) {
            gpio_set_level(LED_GPIO_NUM, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_set_level(LED_GPIO_NUM, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            gpio_set_level(LED_GPIO_NUM, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_GPIO_NUM, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

static void wifi_timeout_monitor_task(void *pvParameter) {
    TickType_t start_time = xTaskGetTickCount();
    const TickType_t timeout = pdMS_TO_TICKS(60000); // 1 minute

    while (1) {
        if (wifi_connected) {
            start_time = xTaskGetTickCount();  // Reset timer
        } else if ((xTaskGetTickCount() - start_time) > timeout) {
            ESP_LOGE(TAG, "Wi-Fi connection failed after 1 minute timeout.");
            wifi_failed = true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t wifi_init(void) {
    ESP_LOGI(TAG, "Initializing Wi-Fi...");
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .scan_method = WIFI_FAST_SCAN,
            .bssid_set = false,
        },
    };

    ESP_LOGI(TAG, "Connecting to SSID: %s", WIFI_SSID);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    // ✅ Start indicator and timeout tasks
    xTaskCreate(&led_wifi_indicator_task, "led_wifi_indicator", 8192, NULL, 5, NULL);
    xTaskCreate(&wifi_timeout_monitor_task, "wifi_timeout_monitor", 2048, NULL, 5, NULL);

    return ESP_OK;
}

bool wifi_is_connected(void) {
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, 0);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

void wifi_wait_for_connection(void) {
    const TickType_t timeout = pdMS_TO_TICKS(30000); // 30 seconds
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, timeout);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi connected.");
    } else {
        ESP_LOGE(TAG, "Timed out waiting for Wi-Fi.");
    }
}