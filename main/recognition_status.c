#include "recognition_status.h"
#include <string.h> // For strncpy
#include "esp_log.h" // For ESP_LOGI

static const char *TAG = "REC_STATUS";

// Define the global status variable
recognition_status_t g_current_recognition_status = {
    .recognized = false,
    .face_id = -1,
    .name = ""
};

// Implementation of the update function
void recognition_status_update(bool recognized, int id, const char* name) {
    // In a real-world scenario, especially if app_main and httpd are separate tasks,
    // you'd use a mutex here to prevent race conditions when reading/writing g_current_recognition_status.
    // Example: xSemaphoreTake(g_status_mutex, portMAX_DELAY);
    //          ... update ...
    //          xSemaphoreGive(g_status_mutex);

    g_current_recognition_status.recognized = recognized;
    g_current_recognition_status.face_id = id;

    if (name) {
        // Ensure null-termination and prevent buffer overflow
        strncpy(g_current_recognition_status.name, name, sizeof(g_current_recognition_status.name) - 1);
        g_current_recognition_status.name[sizeof(g_current_recognition_status.name) - 1] = '\0';
    } else {
        g_current_recognition_status.name[0] = '\0'; // Clear name if no name provided
    }

    ESP_LOGI(TAG, "Status: %s (ID: %d, Name: '%s')",
             recognized ? "Recognized" : "Unknown", id, g_current_recognition_status.name);

    // Release mutex/semaphore here if used
}