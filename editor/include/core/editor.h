#ifndef EDITOR_H
#define EDITOR_H
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include <cimgui.h>
#include <cimgui_impl.h>
#include <cglm/types-struct.h>
#include "../util/types.h"

typedef struct {
    ImGuiContext *context; 
    ImGuiIO *io;

    vec2s mouse;
} Editor;

typedef struct {
    ImTextureID texture_id;
    char *selected_tileset;
    s32 selected_tile;

    s32 rows;
    s32 cols;
    s32 tile_count;
    s32 tile_size;
    ImVec2 image_size;

    vec2s mouse;
    struct { s32 row, col; } mouse_rc;
}TileEditorState;

typedef struct {
    u32 width;
    u32 height;
    s32 rows;
    s32 cols;
    u32 stride;
    u32 point_entity_id;

    vec2s position;
    vec2s mouse;
    struct { s32 row, col; } mouse_rc;

    b8 created;
}CanvasState;

void editor_init(Editor **editor);
void editor_destroy(Editor *editor);
void editor_newframe(void);
void editor_render(void);
#endif
