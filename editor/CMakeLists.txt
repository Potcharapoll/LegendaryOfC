cmake_minimum_required(VERSION 3.11)
project(editor)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ---------------------------- CIMGUI ----------------------------
set(BACKENDS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui/backends/")
set(TABLES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui/imgui_tables.cpp")
set(IMGUI_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/cimgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui/imgui_widgets.cpp
    ${TABLES_SOURCE}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/cimgui/imgui)
add_definitions("-DIMGUI_IMPL_API=extern \"C\"")

list(APPEND IMGUI_SOURCES ${BACKENDS_FOLDER}imgui_impl_opengl3.cpp)
list(APPEND IMGUI_SOURCES ${BACKENDS_FOLDER}imgui_impl_glfw.cpp)

add_library(cimgui SHARED ${IMGUI_SOURCES})
target_link_libraries(cimgui GL glfw)
# ----------------------------------------------------------------
add_subdirectory(./lib/glfw)
add_subdirectory(./lib/cglm)


list(APPEND CFLAGS "-std=c99")
list(APPEND CFLAGS "-O2")
list(APPEND CFLAGS "-g3")
list(APPEND CFLAGS "-Wall")
list(APPEND CFLAGS "-Wextra")
list(APPEND CFLAGS "-Wno-unused-result")
list(APPEND CFLAGS "-Wno-missing-braces")

include_directories(./lib/glfw/include)
include_directories(./lib/cglm/include)
include_directories(./lib/glad/include)
include_directories(./lib/cimgui)
include_directories(./lib/cimgui/generator/output)
include_directories(./lib/stb)
include_directories(./include)

add_executable(${PROJECT_NAME} 
    ./lib/glad/src/glad.c

    ./src/main.c
    ./src/camera.c
    ./src/renderer.c
    ./src/ecs.c
    ./src/window.c
    ./src/editor.c
    ./src/shader.c
    ./src/texture.c
    ./src/vao.c
    ./src/vbo.c
    ./src/array_stack.c
    ./src/array_list.c
)

target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw cglm cimgui m)
