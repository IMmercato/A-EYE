#include "server_comm.h"
#include "config.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "mbedtls/base64.h"

static const char *TAG = "SERVER_COMM";

esp_err_t server_comm_init(void) {
    ESP_LOGI(TAG, "Server communication initialized");
    return ESP_OK;
}

char* encode_image_base64(const uint8_t* image_data, size_t image_len) {
    size_t encoded_len = 0;
    mbedtls_base64_encode(NULL, 0, &encoded_len, image_data, image_len);
    
    char* encoded_data = malloc(encoded_len + 1);
    if (encoded_data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for base64 encoding");
        return NULL;
    }
    
    mbedtls_base64_encode((unsigned char*)encoded_data, encoded_len, &encoded_len, 
                         image_data, image_len);
    encoded_data[encoded_len] = '\0';
    
    return encoded_data;
}

char* server_send_image(camera_fb_t* fb) {
    if (!fb) {
        ESP_LOGE(TAG, "Invalid frame buffer");
        return NULL;
    }

    // Encode image to base64
    char* base64_image = encode_image_base64(fb->buf, fb->len);
    if (!base64_image) {
        return NULL;
    }

    // Create JSON payload
    cJSON *json = cJSON_CreateObject();
    cJSON *image_data = cJSON_CreateString(base64_image);
    cJSON *timestamp = cJSON_CreateNumber(esp_timer_get_time() / 1000);
    cJSON *device_id = cJSON_CreateString("esp32_glasses_001");
    
    cJSON_AddItemToObject(json, "image", image_data);
    cJSON_AddItemToObject(json, "timestamp", timestamp);
    cJSON_AddItemToObject(json, "device_id", device_id);
    
    char* json_string = cJSON_Print(json);
    
    // HTTP client configuration
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = SERVER_TIMEOUT_MS,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_string, strlen(json_string));
    
    // Perform request
    esp_err_t err = esp_http_client_perform(client);
    char* response_buffer = NULL;
    
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);
        
        ESP_LOGI(TAG, "HTTP Status: %d, Content-Length: %d", status, content_length);
        
        if (status == 200 && content_length > 0) {
            response_buffer = malloc(content_length + 1);
            if (response_buffer) {
                esp_http_client_read(client, response_buffer, content_length);
                response_buffer[content_length] = '\0';
            }
        }
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    
    // Cleanup
    esp_http_client_cleanup(client);
    free(base64_image);
    free(json_string);
    cJSON_Delete(json);
    
    return response_buffer;
}