cmake_minimum_required(VERSION 3.14)

project(tiny-renderer)

add_executable(${PROJECT_NAME} Source/main.c
                Dependencies/stb_image_write.c
                Dependencies/tinyobj_loader_c.c
                Dependencies/loader.c)

include_directories(Dependencies)
include_directories(Source)