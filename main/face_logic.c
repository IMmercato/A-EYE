#include "esp_log.h"
#include "esp_camera.h"
#include "dl_image.h"
#include "fb_gfx.h"

static const char *TAG = "FACE_LOGIC";

static dl_matrix3du_t *aligned_face = NULL;

void init_face_logic(void) {
    aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    face_id_init();
    ESP_LOGI(TAG, "Face recognition logic initialized");
}

bool recognize_face(camera_fb_t *frame) {
    if (!frame || !aligned_face) {
        ESP_LOGE(TAG, "Invalid frame or uninitialized face buffer");
        return false;
    }

    // Convert JPEG to RGB888
    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);
    fmt2rgb888(frame->buf, frame->len, frame->format, image_matrix->item);

    // Detect faces
    box_array_t *net_boxes = face_detect(image_matrix, NULL);
    if (!net_boxes || net_boxes->len == 0) {
        ESP_LOGI(TAG, "No face detected");
        dl_matrix3du_free(image_matrix);
        return false;
    }

    // Align face
    if (align_face(net_boxes, image_matrix, aligned_face) != ESP_OK) {
        ESP_LOGE(TAG, "Face alignment failed");
        dl_matrix3du_free(image_matrix);
        return false;
    }

    // Recognize face
    int matched_id = recognize_face_id(aligned_face);
    dl_matrix3du_free(image_matrix);

    if (matched_id >= 0) {
        ESP_LOGI(TAG, "Face recognized: ID %d", matched_id);
        return true;
    } else {
        ESP_LOGI(TAG, "Unknown face");
        return false;
    }
}