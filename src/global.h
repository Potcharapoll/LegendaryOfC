#ifndef GLOBAL_H
#define GLOBAL_H
#include "gfx/window.h"
#include "util/types.h"
#include "core/asset_manager.h"
#include "core/player.h"
#include "core/chunk.h"

struct Global {
    struct Window        *window;
    struct Camera        *camera;
    struct AssetManager  *asset_manager;

    f32   dt;

    struct {
        Chunk *chunk;
        u32   chunk_idx;
        b8    next_chunk;
    } ChunkState;

    struct {
    } DialogState;

    struct {
        vec3s position;
        enum Direction direction;

        u32 animation_id;
        u32 body_id;
    } PlayerState;
};

extern struct Global global;
#endif
