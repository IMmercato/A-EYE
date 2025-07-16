#include <esp_log.h>
#include <esp_http_server.h>
#include "esp_err.h"

static const char *TAG = "httpd";

extern const uint8_t index_html_start[] asm("_binary_data_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_data_index_html_end");

esp_err_t index_handler(httpd_req_t *req) {
    extern const uint8_t index_html_start[] asm("_binary_data_index_html_start");
    extern const uint8_t index_html_end[] asm("_binary_data_index_html_end");

    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        return NULL;
    }

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
};

    httpd_register_uri_handler(server, &index_uri);
    return server;
}