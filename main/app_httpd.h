#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_http_server.h>

// Starts the HTTP server and returns the server handle
httpd_handle_t start_webserver(void);

#ifdef __cplusplus
}
#endif