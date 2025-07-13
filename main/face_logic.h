#ifndef FACE_LOGIC_H
#define FACE_LOGIC_H

#include "esp_camera.h"

void init_face_logic(void);
bool recognize_face(camera_fb_t *frame);

#endif