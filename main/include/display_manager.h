#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "esp_err.h"

esp_err_t display_init(void);
void display_show_text(const char* text);
void display_blink_status(void);
void display_test_pattern(void);

#endif