// Existing includes from app_httpd.c
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "sdkconfig.h" // For CONFIG_ESP_WHO_ENABLE_HTTPD_STATIC_URL
#include <string.h> // For strlen
#include "recognition_status.h" // NEW: Include your recognition status header
#include "esp_spiffs.h" // Needed for SPIFFS

// --- Existing httpd_handle_t variable (usually global or returned) ---
// extern httpd_handle_t camera_httpd; // Often declared in app_httpd.h

static const char *TAG = "app_httpd";

// --- Existing handlers like /stream, /capture, /enroll_face etc. will be here ---
// You will find functions like:
// esp_err_t stream_handler(httpd_req_t *req);
// esp_err_t capture_handler(httpd_req_t *req);
// etc.

// --- NEW: HTTP handler for recognition status JSON ---
esp_err_t recognition_status_handler(httpd_req_t *req) {
    char json_response[128]; // Adjust size as needed, e.g., 256 for longer names
    recognition_status_t status = g_current_recognition_status; // Get current status

    if (status.recognized) {
        snprintf(json_response, sizeof(json_response),
                 "{\"status\":\"recognized\", \"id\":%d, \"name\":\"%s\"}",
                 status.face_id, status.name);
    } else {
        snprintf(json_response, sizeof(json_response),
                 "{\"status\":\"unknown\"}"); // Or "looking" if no face detected
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, strlen(json_response));
    return ESP_OK;
}

// --- NEW: HTTP handler for serving static files from SPIFFS ---
// This function needs to be generic enough to serve index.html, style.css, script.js
esp_err_t serve_static_file_handler(httpd_req_t *req) {
    char filepath[128]; // Max file path length
    sprintf(filepath, "/spiffs%s", req->uri); // Assumes /spiffs is the mount point

    // Determine content type based on file extension
    const char *content_type = "text/plain";
    if (strstr(req->uri, ".html")) content_type = "text/html";
    else if (strstr(req->uri, ".css")) content_type = "text/css";
    else if (strstr(req->uri, ".js")) content_type = "application/javascript";
    else if (strstr(req->uri, ".jpg") || strstr(req->uri, ".jpeg")) content_type = "image/jpeg";
    else if (strstr(req->uri, ".png")) content_type = "image/png";

    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file : %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, content_type);

    char chunk[512]; // Buffer for reading file chunks
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, sizeof(chunk), f);
        if (read_bytes > 0) {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                ESP_LOGE(TAG, "File sending failed!");
                fclose(f);
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // End of stream
    return ESP_OK;
}


// --- Modify the start_httpd function ---
// This function is usually where all URI handlers are registered.
httpd_handle_t start_httpd(void) {
    httpd_handle_t httpd_handle = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16; // Increase if you have many handlers
    config.uri_match_fn = httpd_uri_match_wildcard; // Allow wildcard matching for static files

    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);
    if (httpd_start(&httpd_handle, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");

        // --- Existing handlers (you'll keep these) ---
        // For example:
        // httpd_uri_t stream_uri = { ... }; httpd_register_uri_handler(httpd_handle, &stream_uri);
        // httpd_uri_t capture_uri = { ... }; httpd_register_uri_handler(httpd_handle, &capture_uri);
        // httpd_uri_t enroll_uri = { ... }; httpd_register_uri_handler(httpd_handle, &enroll_uri);

        // --- NEW: Register the recognition status JSON handler ---
        httpd_uri_t status_uri = {
            .uri       = "/recognition_status.json",
            .method    = HTTP_GET,
            .handler   = recognition_status_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(httpd_handle, &status_uri);
        ESP_LOGI(TAG, "Registered /recognition_status.json URI");

        // --- NEW: Register handler for static files from SPIFFS ---
        // This will serve index.html, style.css, script.js, etc.
        httpd_uri_t static_files_uri = {
            .uri       = "/*", // Catch all for other paths
            .method    = HTTP_GET,
            .handler   = serve_static_file_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(httpd_handle, &static_files_uri);
        ESP_LOGI(TAG, "Registered /* for static file serving");


        // Default behavior for root path ("/") usually serves index.html
        // You might have a specific handler for "/", if not, the "/*" handler above will catch it.
        // If there's an existing "/" handler in your esp-who code, make sure it
        // serves the modified index.html, potentially by calling `serve_static_file_handler` for "/index.html".
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = serve_static_file_handler, // Directly serve index.html via the static file handler
            .user_ctx  = "/index.html" // Pass the actual file path
        };
        httpd_register_uri_handler(httpd_handle, &root_uri);
        ESP_LOGI(TAG, "Registered / URI to serve index.html");


    } else {
        ESP_LOGE(TAG, "Error starting HTTP server!");
    }
    return httpd_handle;
}