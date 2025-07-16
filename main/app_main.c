#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_camera.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h" // For LED control (optional)
// ESP-WHO specific includes (these should already be there)
#include "app_httpd.h"      // For starting the web server
#include "app_camera.h"     // For camera initialization
#include "face_detection.h" // For face detection models
#include "face_recognition.h" // For face recognition models
#include "face_enroll.h"    // For face enrollment
// #include "dl_image.h" // Might be needed for image matrix operations
// #include "esp_who_version.h" // Optional
#include "recognition_status.h" // NEW: Include your recognition status header
#include "esp_spiffs.h" // NEW: Include SPIFFS header for initialization

static const char *TAG = "app_main";

// --- Function prototypes (existing in app_main.c) ---
// extern void start_httpd(void);
// extern void camera_init(void);
// extern void start_wifi(void);

// Helper to get enrolled face name (might exist in esp-who, or you'll create a simple one)
// If esp-who has an internal mapping, use that. Otherwise, you can
// implement a simple one for demo purposes (e.g., "User 0", "User 1").
// For full functionality, you'd integrate with ESP-WHO's enrollment database.
const char* get_enrolled_face_name(int id) {
    // This is a placeholder. In a real esp-who app, you'd fetch this from
    // the enrollment database.
    static char name_buffer[32];
    switch (id) {
        case 0:  return "Alice";
        case 1:  return "Bob";
        case 2:  return "Charlie";
        default:
            snprintf(name_buffer, sizeof(name_buffer), "User %d", id);
            return name_buffer;
    }
}


// --- Main Application Loop/Task ---
// This function or a similar FreeRTOS task is where the camera frame is
// processed for face detection and recognition.
void face_detection_recognition_task(void *pvParameters) {
    camera_fb_t *fb = NULL;
    dl_matrix3d_t *image_matrix = NULL;

    while (1) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        // Convert camera frame to DL image matrix (required by ESP-WHO models)
        // This line or similar will be present:
        image_matrix = dl_matrix3d_alloc(1, fb->width, fb->height, 3); // For RGB888
        if (fb->format == PIXFORMAT_JPEG) {
            // Decompress JPEG to RGB for face detection if camera is sending JPEG
            bool converted = fmt2rgb888(fb->buf, fb->len, fb->width, fb->height, image_matrix->item);
            if (!converted) {
                ESP_LOGE(TAG, "JPEG to RGB conversion failed");
                dl_matrix3d_free(image_matrix);
                esp_camera_fb_return(fb);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                continue;
            }
        } else if (fb->format == PIXFORMAT_RGB565) {
             // If camera directly outputs RGB565, convert to RGB888
             // ESP-WHO models usually expect RGB888 or similar
             // This conversion is more complex, ensure your esp-who version handles it
             // or directly configure camera to RGB888 if possible (less common for OV2640).
             ESP_LOGW(TAG, "RGB565 to RGB888 conversion not fully implemented in this example.");
             // For simplicity, let's assume direct RGB888 or JPEG conversion works.
        } else {
             // Handle other formats or assume direct use if model supports it
             ESP_LOGE(TAG, "Unsupported pixel format: %d", fb->format);
             dl_matrix3d_free(image_matrix);
             esp_camera_fb_return(fb);
             vTaskDelay(100 / portTICK_PERIOD_MS);
             continue;
        }


        // --- Face Detection ---
        face_info_t *face_info = dl_face_detect(image_matrix); // This is where detection happens
        if (face_info && face_info->num > 0) {
            // Face(s) detected, now try to recognize the first one
            dl_matrix3d_t *aligned_face = NULL;
            int matched_id = -1;
            float similarity = 0.0;
            const char *matched_name = "Unknown";

            // Assume we process the first detected face for simplicity
            if (face_info->num > 0) {
                aligned_face = dl_face_align(image_matrix, face_info->box[0], &matched_id); // Align face for recognition
                if (aligned_face) {
                    face_id_recognize(aligned_face, &matched_id, &similarity); // Perform recognition

                    if (matched_id != -1) {
                        matched_name = get_enrolled_face_name(matched_id);
                        recognition_status_update(true, matched_id, matched_name);
                    } else {
                        recognition_status_update(false, -1, NULL); // Unknown face
                    }
                    dl_matrix3d_free(aligned_face);
                } else {
                    recognition_status_update(false, -1, NULL); // Face alignment failed
                }
            } else {
                 recognition_status_update(false, -1, NULL); // No faces detected
            }

        } else {
            // No face detected in the frame
            recognition_status_update(false, -1, NULL); // Update status to "unknown/no face"
        }

        dl_matrix3d_free(image_matrix);
        esp_camera_fb_return(fb);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Adjust delay as needed for desired FPS
    }
}

// --- SPIFFS Initialization Function ---
void init_spiffs(void) {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs", // Mount point for SPIFFS
      .partition_label = NULL, // Use default partition labeled "storage" or similar
      .max_files = 5,          // Max number of files that can be open at once
      .format_if_mount_failed = true // Format partition if mount fails
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format SPIFFS filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}


// --- app_main function (entry point) ---
void app_main(void) {
    // Initialize NVS (Non-Volatile Storage) for Wi-Fi and other settings
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize SPIFFS
    init_spiffs(); // Ensure SPIFFS is mounted before web server starts

    // Initialize Wi-Fi
    start_wifi(); // This function should be present in esp-who example

    // Initialize Camera
    camera_init(); // This function should be present in esp-who example

    // Start HTTP Server
    // The start_httpd function will also register your new URI handler
    start_httpd();

    // Create a FreeRTOS task for face detection and recognition
    xTaskCreatePinnedToCore(&face_detection_recognition_task, "face_rec_task", 4 * 1024, NULL, 5, NULL, 0); // Core 0 for camera/AI
    // Adjust stack size (e.g., 4*1024 or higher) based on your needs and available memory
    // Adjust priority (5) if needed
}