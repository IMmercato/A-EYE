#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_spiffs.h>
#include <string.h>
#include <sys/stat.h>
#include "app_httpd.h"

#define MAX_FILENAME 512

static const char *TAG = "HTTPD";

// Initialize SPIFFS
esp_err_t init_spiffs(void) {
    ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "www",
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        return ret;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info("www", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS: %d kB total, %d kB used", total / 1024, used / 1024);
    }
    
    return ESP_OK;
}

// Get MIME type from file extension
const char* get_mime_type(const char* filename) {
    if (strstr(filename, ".html")) return "text/html";
    if (strstr(filename, ".css")) return "text/css";
    if (strstr(filename, ".js")) return "application/javascript";
    if (strstr(filename, ".png")) return "image/png";
    if (strstr(filename, ".jpg") || strstr(filename, ".jpeg")) return "image/jpeg";
    if (strstr(filename, ".ico")) return "image/x-icon";
    return "text/plain";
}

// Generic file handler
esp_err_t file_handler(httpd_req_t *req) {
    char filepath[7 + MAX_FILENAME + 1];
    const char* filename = req->uri;
    
    // Default to index.html for root
    if (strcmp(filename, "/") == 0) {
        filename = "/index.html";
    }
    
    snprintf(filepath, sizeof(filepath), "/spiffs%s", filename);
    
    FILE *file = fopen(filepath, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // Set content type
    httpd_resp_set_type(req, get_mime_type(filename));
    
    // Send file in chunks
    char buffer[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(buffer, 1, sizeof(buffer), file);
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, buffer, read_bytes);
        }
    } while (read_bytes > 0);
    
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    fclose(file);
    
    return ESP_OK;
}

// API endpoint example
esp_err_t api_status_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    const char* json_response = "{\"status\":\"online\",\"uptime\":12345,\"memory\":\"75%\"}";
    httpd_resp_send(req, json_response, strlen(json_response));
    return ESP_OK;
}

// Start webserver with SPIFFS support
httpd_handle_t start_webserver(void) {
    // Initialize SPIFFS first
    if (init_spiffs() != ESP_OK) {
        return NULL;
    }
    
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    
    ESP_LOGI(TAG, "Starting HTTP server on port: %d", config.server_port);
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return NULL;
    }
    
    // Register handlers
    httpd_uri_t file_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = file_handler,
        .user_ctx = NULL
    };
    
    httpd_uri_t api_status_uri = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = api_status_handler,
        .user_ctx = NULL
    };
    
    httpd_register_uri_handler(server, &api_status_uri);
    httpd_register_uri_handler(server, &file_uri);
    
    ESP_LOGI(TAG, "HTTP server started");
    return server;
}