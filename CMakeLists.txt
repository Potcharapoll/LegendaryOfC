cmake_minimum_required(VERSION 3.11)
project(legendaryofc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
add_subdirectory(lib/freetype)

include_directories(lib/glfw/include)
include_directories(lib/cglm/include)
include_directories(lib/glad/include)
include_directories(lib/freetype)
include_directories(lib/stb)

add_executable(${PROJECT_NAME} 
    lib/glad/src/glad.c

    src/gfx/window.c
    src/gfx/shader.c
    src/gfx/texture.c
    src/gfx/spritesheet.c
    
    src/core/animation.c
    src/core/camera.c
    src/core/renderer.c
    src/core/asset_manager.c
    src/core/chunk.c
    src/core/physics.c
    src/core/batch_render.c
    src/core/player.c
    src/core/prefab.c

    src/util/hashtable.c
    src/util/array_list.c

    src/main.c
)

target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw cglm freetype m)
