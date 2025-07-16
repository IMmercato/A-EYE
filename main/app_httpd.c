#include <esp_log.h>
#include <esp_http_server.h>
#include <string.h>
#include "app_httpd.h"

static const char *TAG = "HTTPD";

// Simple HTML content as string literal
static const char index_html[] = 
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>A-EYE Control Panel</title>\n"
"    <style>\n"
"        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }\n"
"        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n"
"        h1 { color: #333; text-align: center; }\n"
"        .status { padding: 15px; margin: 20px 0; border-radius: 5px; }\n"
"        .status.online { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }\n"
"        .controls { display: flex; gap: 10px; justify-content: center; margin: 20px 0; }\n"
"        button { padding: 10px 20px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }\n"
"        .btn-primary { background-color: #007bff; color: white; }\n"
"        .btn-success { background-color: #28a745; color: white; }\n"
"        .btn-danger { background-color: #dc3545; color: white; }\n"
"        button:hover { opacity: 0.8; }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class=\"container\">\n"
"        <h1>🤖 A-EYE System</h1>\n"
"        <div class=\"status online\">\n"
"            <strong>Status:</strong> System Online\n"
"        </div>\n"
"        <div class=\"controls\">\n"
"            <button class=\"btn-primary\" onclick=\"alert('Feature coming soon!')\">Start Monitoring</button>\n"
"            <button class=\"btn-success\" onclick=\"alert('Feature coming soon!')\">Settings</button>\n"
"            <button class=\"btn-danger\" onclick=\"alert('Feature coming soon!')\">Stop</button>\n"
"        </div>\n"
"        <div style=\"text-align: center; margin-top: 30px; color: #666;\">\n"
"            <p>A-EYE Control Panel v1.0</p>\n"
"        </div>\n"
"    </div>\n"
"</body>\n"
"</html>";

esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, strlen(index_html));
    return ESP_OK;
}

// Starts the webserver
httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    ESP_LOGI(TAG, "Starting HTTP server on port: %d", config.server_port);
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return NULL;
    }

    // Register URI handlers
    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL
    };
    
    httpd_register_uri_handler(server, &index_uri);
    ESP_LOGI(TAG, "HTTP server started");
    return server;
}