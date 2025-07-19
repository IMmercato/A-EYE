#include "server_comm.h"
#include "config.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "mbedtls/base64.h"

static const char *TAG = "SERVER_COMM";
static esp_http_client_handle_t s_http_client = NULL; // Make it static global

esp_err_t server_comm_init(void) {
    ESP_LOGI(TAG, "Server communication initializing...");
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST, // Default method, can be overridden per request
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = SERVER_TIMEOUT_MS,
        .event_handler = NULL,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .skip_cert_common_name_check = false,
        .keep_alive_enable = true, // Enable HTTP Keep-Alive
    };

    s_http_client = esp_http_client_init(&config);
    if (s_http_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Server communication initialized successfully");
    return ESP_OK;
}

esp_err_t server_comm_deinit(void) {
    if (s_http_client != NULL) {
        esp_err_t err = esp_http_client_cleanup(s_http_client);
        s_http_client = NULL;
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to cleanup HTTP client: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG, "HTTP client cleaned up");
    }
    return ESP_OK;
}

char* encode_image_base64(const uint8_t *image_data, size_t image_len) {
    size_t encoded_len = 0;
    mbedtls_base64_encode(NULL, 0, &encoded_len, image_data, image_len);

    char* encoded_data = malloc(encoded_len + 1);
    if (encoded_data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for base64 encoding");
        return NULL;
    }

    if (mbedtls_base64_encode((unsigned char*)encoded_data, encoded_len, &encoded_len,
                              image_data, image_len) != 0) {
        ESP_LOGE(TAG, "Base64 encoding failed");
        free(encoded_data);
        return NULL;
    }

    encoded_data[encoded_len] = '\0';
    return encoded_data;
}

char* server_send_image(camera_fb_t *fb) {
    if (!fb || !fb->buf || fb->len == 0) {
        ESP_LOGE(TAG, "Invalid frame buffer");
        return NULL;
    }
    if (s_http_client == NULL) {
        ESP_LOGE(TAG, "HTTP client not initialized. Call server_comm_init() first.");
        return NULL;
    }

    char* base64_image = encode_image_base64(fb->buf, fb->len);
    if (!base64_image) {
        ESP_LOGE(TAG, "Failed to encode image to base64");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "image", cJSON_CreateString(base64_image));
    cJSON_AddItemToObject(json, "timestamp", cJSON_CreateNumber(esp_timer_get_time() / 1000));
    cJSON_AddItemToObject(json, "device_id", cJSON_CreateString("esp32_glasses_001"));

    char *json_string = cJSON_Print(json);
    if (!json_string) {
        ESP_LOGE(TAG, "Failed to convert JSON to string");
        cJSON_Delete(json);
        free(base64_image);
        return NULL;
    }

    // Reset URL and method for the current request (if needed, though POST to same URL is default)
    esp_http_client_set_url(s_http_client, SERVER_URL);
    esp_http_client_set_method(s_http_client, HTTP_METHOD_POST);
    esp_http_client_set_header(s_http_client, "Content-Type", "application/json");
    esp_http_client_set_post_field(s_http_client, json_string, strlen(json_string));

    esp_err_t err = esp_http_client_perform(s_http_client);
    char* response_buffer = NULL;

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(s_http_client);
        int content_length = esp_http_client_get_content_length(s_http_client);

        ESP_LOGI(TAG, "HTTP Status: %d, Content-Length: %d", status, content_length);

        if (status == 200 && content_length > 0) {
            response_buffer = malloc(content_length + 1);
            if (response_buffer) {
                int read = esp_http_client_read(s_http_client, response_buffer, content_length);
                if (read > 0) {
                    response_buffer[read] = '\0';
                }
                else {
                    free(response_buffer);
                    response_buffer = NULL;
                }
            }
        }
    }
    else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    // No cleanup of the client handle here, only data specific to this request
    free(base64_image);
    free(json_string);
    cJSON_Delete(json);

    return response_buffer;
}