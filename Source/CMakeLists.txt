add_executable(${PROJECT_NAME} main.c
                Window.c
                stb_image_write.c
                tinyobj_loader_c.c
                loader.c)

target_link_libraries(${PROJECT_NAME}
PRIVATE
GLAD
glfw)

target_include_directories(${PROJECT_NAME}
PRIVATE
"${PROJECT_SOURCE_DIR}/Dependencies")