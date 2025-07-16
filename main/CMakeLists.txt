idf_component_register(
    SRCS "app_main.c" "app_httpd.c"
    INCLUDE_DIRS "."
    EMBED_FILES "data/index.html"
)