#ifndef GLOBAL_H
#define GLOBAL_H
#include "engine/editor.h"

#include "gfx/window.h"

#include "util/types.h"

#include "core/asset_manager.h"
#include "core/physics.h"
#include "core/player.h"
#include "core/chunk.h"

enum CursorMode {
    START_POINT,
    END_POINT,
    NORMAL
};

enum FadeState {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
};

struct Global {
    struct Window        *window;
    struct Camera        *camera;
    struct AssetManager  *asset_manager;

    f32 dt;

    struct {
        Chunk *chunk;
        Chunks chunk_id;
    } ChunkState;

    struct {
        char *name;
        struct DialogNode *curr_dialog_node;

        u8 selected_answer;
    } DialogState;

    struct {
        enum Direction direction;
        u32            animation_id;
        u32            body_id;
    } PlayerState;

    struct {
        f32 alpha;
        enum FadeState state;
    } FadeState;

    // editor/debugging
    struct {
        struct ImGui *editor;
        vec2 start_point, end_point;
        enum CursorMode cursor_mode;

        void(*collision_callback)(Static_Body* body, Body *other);

        b8 toggle_editor;
        b8 toggle_collision;
        b8 toggle_show_collider;
    };
};

extern struct Global global;
#endif
