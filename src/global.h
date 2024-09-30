#ifndef GLOBAL_H
#define GLOBAL_H

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

enum GameAct {
    GAME_ACT1,
    GAME_ACT2,
    GAME_ACT3,
    GAME_ACT4
};

struct Global {
    struct Window *window;
    struct AssetManager *asset_manager;
    Scene *scene;
    Physics *physics;
    Animation *animations;
    Timer *timer;

    f32 dt;
    f32 input_delay;
    pthread_mutex_t lock;

    struct {
        enum GameAct act;
    } GameState;

    struct {
        enum Direction direction;
        u32 animation_id;
        u32 body_id;
    } PlayerState;

    void   (*collision_callback)(Static_Body* body, Body *other);
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
