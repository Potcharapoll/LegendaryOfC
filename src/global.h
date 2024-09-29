#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef DEBUG
#include "engine/editor.h"
#endif

#include "gfx/window.h"

#include "util/types.h"

#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/physics.h"
#include "core/player.h"
#include "core/scnce.h"
#include "core/timer.h"

#include <pthread.h>

enum CursorMode {
    START_POINT,
    END_POINT,
    NORMAL
};

// Introduction (ACT0)
enum GameAct {
    ACT0,
    ACT1,
    ACT2,
    ACT3,
    ACT4
};

struct Global {
    struct Window *window;
    struct AssetManager *asset_manager;
    Scnce *scnce;
    Physics *physics;
    Animation *animations;
    Timer *timer;

    f32 dt;
    f32 input_delay;

    vec4s gradient;
    enum GameAct act;
    pthread_mutex_t lock;

    struct {
        char *name;
        struct DialogNode *curr_dialog_node;

        u8 selected_answer;
    } DialogState;

    struct {
        enum Direction direction;
        u32 animation_id;
        u32 body_id;

        b8 on_collision;
    } PlayerState;

    void(*collision_callback)(Static_Body* body, Body *other);

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
