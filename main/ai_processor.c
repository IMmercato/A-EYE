#include "ai_processor.h"
#include "wifi_manager.h"
#include "camera_manager.h"
#include "server_comm.h"
#include "display_manager.h"
#include "config.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "AI_PROCESSOR";

esp_err_t ai_processor_init(void) {
    ESP_LOGI(TAG, "AI processor initialized");
    return ESP_OK;
}

void process_server_response(const char* response) {
    if (!response) {
        ESP_LOGE(TAG, "Empty response");
        display_show_text("Empty response");
        return;
    }

    char preview[64];
    snprintf(preview, sizeof(preview), "Resp: %.57s", response);
    display_show_text(preview);
    
    cJSON *json = cJSON_Parse(response);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON response");
        return;
    }
    
    // Process recognized faces
    cJSON *recognized_faces = cJSON_GetObjectItem(json, "recognized_faces");
    if (cJSON_IsArray(recognized_faces)) {
        int array_size = cJSON_GetArraySize(recognized_faces);
        for (int i = 0; i < array_size; i++) {
            cJSON *face = cJSON_GetArrayItem(recognized_faces, i);
            cJSON *name = cJSON_GetObjectItem(face, "name");
            cJSON *confidence = cJSON_GetObjectItem(face, "confidence");
            
            if (cJSON_IsString(name) && cJSON_IsNumber(confidence)) {
                char display_text[64];
                snprintf(display_text, sizeof(display_text), "Hello %s! (%.0f%%)", 
                        name->valuestring, confidence->valuedouble * 100);
                display_show_text(display_text);
                ESP_LOGI(TAG, "Recognized: %s", display_text);
            }
        }
    }
    
    // Process unknown faces
    cJSON *unknown_faces = cJSON_GetObjectItem(json, "unknown_faces");
    if (cJSON_IsNumber(unknown_faces) && unknown_faces->valueint > 0) {
        display_show_text("Unknown person");
        ESP_LOGI(TAG, "Unknown faces detected: %d", unknown_faces->valueint);
    }
    
    // Process objects
    cJSON *objects = cJSON_GetObjectItem(json, "objects");
    if (cJSON_IsArray(objects)) {
        int array_size = cJSON_GetArraySize(objects);
        for (int i = 0; i < array_size; i++) {
            cJSON *obj = cJSON_GetArrayItem(objects, i);
            cJSON *name = cJSON_GetObjectItem(obj, "name");
            cJSON *confidence = cJSON_GetObjectItem(obj, "confidence");
            
            if (cJSON_IsString(name) && cJSON_IsNumber(confidence)) {
                char display_text[64];
                snprintf(display_text, sizeof(display_text), "Object: %s", name->valuestring);
                display_show_text(display_text);
                ESP_LOGI(TAG, "Object detected: %s (%.0f%%)", 
                        name->valuestring, confidence->valuedouble * 100);
            }
        }
    }
    
    // Process contextual information
    cJSON *context = cJSON_GetObjectItem(json, "context");
    if (cJSON_IsString(context)) {
        display_show_text(context->valuestring);
        ESP_LOGI(TAG, "Context: %s", context->valuestring);
    }
    
    cJSON_Delete(json);
}

void ai_processing_task(void* pvParameters) {
    ESP_LOGI(TAG, "AI processing task started");
    
    while (1) {
        // Wait for WiFi connection
        if (!wifi_is_connected()) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Capture frame
        ESP_LOGI(TAG, "Attempting to capture frame...");
        camera_fb_t* fb = camera_capture_frame();
        if (fb) {
            ESP_LOGI(TAG, "Captured frame! Size: %d bytes", fb->len);
        } else {
            ESP_LOGE(TAG, "Camera capture failed");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Indicate capture
        display_blink_status();
        
        // Send to server
        char* response = server_send_image(fb);
        camera_return_frame(fb);
        
        // Process response
        if (response) {
            process_server_response(response);
            free(response);
        }
        
        // Wait before next capture
        vTaskDelay(CAPTURE_INTERVAL_MS / portTICK_PERIOD_MS);
    }
}