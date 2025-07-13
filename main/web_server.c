#include "esp_http_server.h"
#include "esp_camera.h"
#include "face_logic.h"
#include "esp_log.h"

esp_err_t stream_handler(httpd_req_t *req) {
        camera_fb_t *frame = esp_camera_fb_get();
    if (!frame) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    bool known = recognize_face(frame);
    const char *response = known ? "Hello, Imesh!" : "WHO?";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    esp_camera_fb_return(frame);
    return ESP_OK;
}

void start_web_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri = "/face",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);
    } else {
        ESP_LOGE("WEB_SERVER", "Failed to start web server");
    }
}