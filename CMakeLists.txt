cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(A-EYE)

spiffs_create_partition_image(www main/www FLASH_IN_PROJECT)