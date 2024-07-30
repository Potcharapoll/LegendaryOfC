#ifndef EDITOR_H
#define EDITOR_H
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include "cimgui.h"
#include "cimgui_impl.h"
#include "types.h"

typedef struct {
    ImGuiContext *context; 
    ImGuiIO *io;

    f32 mousex, mousey;
} Editor;

typedef struct {
    char *selected_tileset;
    s32 selected_tile;
    ImTextureID textureId;

    ImVec2 imageSize;
    s32 rowsCount, colsCount;
    s32 tileCount, tileSize;

    f32 mousex, mousey;
    s32 mouse_row, mouse_col;
}TileEditorState;

typedef struct {
    u32 width, height;
    s32 rows, cols;
    u32 size;
    b8 created;

    f32 posx, posy;
    f32 mousex, mousey;
    s32 mouse_row, mouse_col;

    u32 pointed_entity_id;
}CanvasState;

void editor_init(Editor **editor);
void editor_destroy(Editor *editor);
void editor_newframe(void);
void editor_render(void);
#endif
