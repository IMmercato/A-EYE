#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "esp_err.h"
#include "esp_camera.h"

esp_err_t camera_init(void);
camera_fb_t* camera_capture_frame(void);
void camera_return_frame(camera_fb_t* fb);

#endif