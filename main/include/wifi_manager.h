#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>

esp_err_t wifi_init(void);
bool wifi_is_connected(void);
void wifi_wait_for_connection(void);

#endif