idf_component_register(
    SRCS "app_main.c" "ai_processor.c" "camera_manager.c" "display_manager.c" "server_comm.c" "wifi_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES esp32-camera esp_wifi esp_timer esp_http_client esp_psram json mbedtls driver nvs_flash
)