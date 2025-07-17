#ifndef AI_PROCESSOR_H
#define AI_PROCESSOR_H

#include "esp_err.h"

esp_err_t ai_processor_init(void);
void process_server_response(const char* response);
void ai_processing_task(void* pvParameters);

#endif