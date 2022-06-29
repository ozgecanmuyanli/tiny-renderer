add_library(GLAD
STATIC
glad.c)

target_include_directories(GLAD
PUBLIC
"${PROJECT_SOURCE_DIR}/Dependencies")