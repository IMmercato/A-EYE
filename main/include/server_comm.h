#ifndef SERVER_COMM_H
#define SERVER_COMM_H

#include "esp_err.h"
#include "esp_camera.h"

esp_err_t server_comm_init(void);
esp_err_t server_comm_deinit(void);
char* server_send_image(camera_fb_t* fb);

#endif