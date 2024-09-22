#ifndef GLOBAL_H
#define GLOBAL_H
#include "gfx/window.h"
#include "util/types.h"
#include "core/asset_manager.h"
#include "core/player.h"
#include "core/chunk.h"

typedef enum {
    FREE,
    ON_DIALOG
} GameState;

struct Global {
    struct Window        *window;
    struct Camera        *camera;
    struct AssetManager  *asset_manager;

    f32 dt;
    GameState game_state;

    struct {
        Chunk  *chunk;
        Chunks chunk_id;
    } ChunkState;

    struct {
        char   *name;
        struct DialogNode *curr_dialog_node;

        u8 selected_answer;
    } DialogState;

    struct {
        enum Direction direction;
        u32            animation_id;
        u32            body_id;
    } PlayerState;


    // debugging
    b8 toggle_collision;
    b8 toggle_show_collider;
};

extern struct Global global;
#endif
