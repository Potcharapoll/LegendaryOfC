cmake_minimum_required(VERSION 3.11)
project(legendaryofc VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type (Debug or Release)" FORCE)
endif()
 
add_subdirectory(lib/glfw)
add_subdirectory(lib/cglm)

include_directories(lib/glfw/include)
include_directories(lib/cglm/include)
include_directories(lib/glad/include)
include_directories(lib/stb)

list(APPEND LIB glfw)
list(APPEND LIB cglm)
list(APPEND LIB m)

add_compile_options(-Wno-missing-braces -Wno-unused-result -Wno-unused-parameter)

if (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)

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

    add_compile_options(-g3 -Wall -Wextra)
    message("Configuring LegendaryOfC in Debug")

    include_directories(lib/cimgui)
    include_directories(lib/cimgui/generator/output)
    add_definitions(-DDEBUG)

    list(APPEND LIB cimgui)
else()
    add_compile_options(-O2)
    message("Configuring LegendaryOfC in Release")


endif()

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
    src/core/scene.c
    src/core/timer.c
    src/core/game.c

    src/engine/logger.c
    src/engine/editor.c
    src/engine/editor_collider.c
    src/engine/editor_debug.c

    src/util/hashtable.c
    src/util/array_list.c

    src/main.c
    src/global.c
)    

target_link_options(${PROJECT_NAME} PRIVATE -Wno-missing-braces)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB})
