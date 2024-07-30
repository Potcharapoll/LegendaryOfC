#ifndef GLOBAL_H
#define GLOBAL_H
#include "window.h"
#include "renderer.h"
#include "editor.h"
#include "camera.h"

typedef struct {
    struct Window *window;
    Renderer *renderer;
    Camera *camera;

    struct {
        Editor *editor;
        CanvasState canvas_state;
        TileEditorState tile_editor_state;

        b8 visible_tile_editor;
        b8 visible_canvas_dialog;
        b8 visible_debug_info;
    } editor_state;

    struct {
        vec2s projection_size;
    } settings;

    b8 tile_update;
    vec2s ortho_mouse;
} Global;

extern Global global;
#endif
