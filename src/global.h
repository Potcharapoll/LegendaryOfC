#ifndef GLOBAL_H
#define GLOBAL_H

#include "util/hashtable.h"
#pragma GCC diagnostic ignored "-Wmissing-braces"
#ifdef DEBUG
#include "engine/editor.h"
#endif

#include "gfx/window.h"

#include "util/types.h"

#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/physics.h"
#include "core/player.h"
#include "core/scene.h"
#include "core/timer.h"

#include <pthread.h>

enum CursorMode {
    CURSOR_MODE_START_POINT,
    CURSOR_MODE_END_POINT,
    CURSOR_MODE_NORMAL,

    CURSOR_MODE_LAST
};

struct Global {
    struct Window       *window;
    struct AssetManager *asset_manager;
    Scene               *scene;
    Physics             *physics;
    Animation           *animations;
    Timer               *timer;
    hash_table_t        *dialogs;

    f32 dt;
    f32 input_delay;
    u32 game_state_flag;
    pthread_mutex_t lock;

    struct {
        enum Direction direction;
        u32 animation_id;
        u32 body_id;
    } PlayerState;

    void   (*dialog_callback)(Static_Body* body, Body *other);
    void   (*teleporter_callback)(Static_Body* body, Body *other);
    ivec2s (*get_char_coord)(char c);

#ifdef DEBUG
    struct {
        struct ImGui *editor;
        vec2 start_point, end_point;
        enum CursorMode cursor_mode;

        b8 toggle_editor;
        b8 toggle_collision;
        b8 toggle_show_collider;

        b8 reload_chunk;
        b8 reset_chunk;
    };
#endif
};

extern struct Global global;
#endif
