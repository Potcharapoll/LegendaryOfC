#ifndef GLOBAL_H
#define GLOBAL_H
#include "gfx/window.h"
#include "core/renderer.h"
#include "core/editor.h"
#include "core/camera.h"
#include "core/asset_manager.h"

typedef struct {
    struct Window *window;
    Renderer *renderer;
    Camera *camera;
    asset_manager_t *asset_manager;

    struct {
        Editor *editor;
        CanvasState canvas_state;
        TileEditorState tile_editor_state;

        b8 visible_tile_editor;
        b8 visible_canvas_dialog;
        b8 visible_debug_info;
        b8 visible_asset_manager;
    } editor_state;

    struct {
        vec2s projection_size;
    } settings;

    b8    tile_update;
    vec2s ortho_mouse;
} Global;

extern Global global;
#endif
