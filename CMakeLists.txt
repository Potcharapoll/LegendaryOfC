cmake_minimum_required(VERSION 3.11)
project(legendaryofc VERSION 1.0.0)

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


list(APPEND CFLAGS "-Wall")
list(APPEND CFLAGS "-Wextra")
list(APPEND CFLAGS "-std=c99")
list(APPEND CFLAGS "-g3")
list(APPEND CFLAGS "-O2")
list(APPEND CFLAGS "-Wno-missing-braces")
list(APPEND CFLAGS "-Wno-unused-parameter")
list(APPEND CFLAGS "-Wno-unused-result")

add_subdirectory(lib/glfw)
add_subdirectory(lib/cglm)

include_directories(lib/glfw/include)
include_directories(lib/cglm/include)
include_directories(lib/glad/include)
include_directories(lib/cimgui)
include_directories(lib/cimgui/generator/output)
include_directories(lib/stb)

add_executable(${PROJECT_NAME} 
    lib/glad/src/glad.c

    src/gfx/window.c
    src/gfx/shader.c
    src/gfx/texture.c
    src/gfx/spritesheet.c
    
    src/core/asset_manager.c
    src/core/animation.c
    src/core/physics.c
    src/core/renderer.c
    src/core/camera.c
    src/core/chunk.c
    src/core/player.c
    src/core/prefab.c
    src/core/dialog.c

    src/engine/logger.c
    src/engine/editor.c
    src/engine/editor_collider.c
    src/engine/editor_debug.c

    src/util/hashtable.c
    src/util/array_list.c

    src/main.c
    src/global.c
)

target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw cglm cimgui m)
