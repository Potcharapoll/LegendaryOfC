#include "chunk.h"
#include "batch_render.h"

#include "../util/array_list.h"
#include "../global.h"
#include "../defs.h"
#include "physics.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct BatchRender *_chunk_batch = NULL;
static array_list *_chunk_list          = NULL;

void chunk_init(void) {
    _chunk_batch = batch_render_init();
    _chunk_list  = array_list_init(sizeof(Chunk), 4);
}

void chunk_destroy(void) {
    batch_render_destroy(_chunk_batch);

    for (u32 id = 0; id < _chunk_list->len; id++) {
        Chunk *chunk = chunk_get(id);
        free(chunk->tilemap);
    }

    array_list_destroy(_chunk_list);
}

Chunk* chunk_get(u64 chunk_id) {
    return array_list_get(_chunk_list, chunk_id);
}

u64 chunk_load_from_file(char *path) {
    FILE *stream;
    Chunk chunk;

    stream = fopen(path, "rb");
    fscanf(stream, "%f %f", &chunk.position.x, &chunk.position.y);
    fscanf(stream, "%u %u", &chunk.spawn.x, &chunk.spawn.y);

    chunk.tilemap    = malloc(CHUNK_SIZE_X * CHUNK_SIZE_Y * sizeof(*chunk.tilemap));
    chunk.position.z = chunk.position.x + (TILE_SIZE * CHUNK_SIZE_X);
    chunk.position.w = chunk.position.y + (TILE_SIZE * CHUNK_SIZE_Y);

    size_t idx = 0;
    for (int i = 0; i < CHUNK_SIZE_Y; ++i) {
        idx = i * CHUNK_SIZE_X;
        fscanf(stream, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u",
            &chunk.tilemap[idx+0].uv,  &chunk.tilemap[idx+1].uv,  &chunk.tilemap[idx+2].uv,  &chunk.tilemap[idx+3].uv,
            &chunk.tilemap[idx+4].uv,  &chunk.tilemap[idx+5].uv,  &chunk.tilemap[idx+6].uv,  &chunk.tilemap[idx+7].uv,
            &chunk.tilemap[idx+8].uv,  &chunk.tilemap[idx+9].uv,  &chunk.tilemap[idx+10].uv, &chunk.tilemap[idx+11].uv,
            &chunk.tilemap[idx+12].uv, &chunk.tilemap[idx+13].uv, &chunk.tilemap[idx+14].uv, &chunk.tilemap[idx+15].uv,
            &chunk.tilemap[idx+16].uv, &chunk.tilemap[idx+17].uv, &chunk.tilemap[idx+18].uv, &chunk.tilemap[idx+19].uv,
            &chunk.tilemap[idx+20].uv, &chunk.tilemap[idx+21].uv, &chunk.tilemap[idx+22].uv, &chunk.tilemap[idx+23].uv,
            &chunk.tilemap[idx+24].uv, &chunk.tilemap[idx+25].uv, &chunk.tilemap[idx+26].uv, &chunk.tilemap[idx+27].uv,
            &chunk.tilemap[idx+28].uv, &chunk.tilemap[idx+29].uv);
    }

    // Make teleport callback function
    u32 collision = 0;
    for (int i = 0; i < (CHUNK_SIZE_X*CHUNK_SIZE_Y); ++i) {
        fscanf(stream, "%u ", &collision);

        if (collision > 0) {
            u32 x = i % CHUNK_SIZE_X;
            u32 y = i / CHUNK_SIZE_X;

            if (collision == 1) {
                chunk.tilemap[i].static_body_id = physics_static_body_create((vec2s){TILE_SIZE * x, TILE_SIZE * y}, DEFAULT_SCALE, COLLISION_PLAYER, COLLISION_SOLID, NULL);
            }
            else if (collision == 2) {
                chunk.tilemap[i].static_body_id = physics_static_body_create((vec2s){TILE_SIZE * x, TILE_SIZE * y}, DEFAULT_SCALE, COLLISION_PLAYER, COLLISION_TELEPORTER, NULL);
            }
        }
        else {
            chunk.tilemap[i].static_body_id = -1;
            chunk.tilemap[i].teleport_id    = -1;
        }
    }

    fclose(stream);
    array_list_append(_chunk_list, &chunk);
    return _chunk_list->len - 1;

}

u64 chunk_create(vec2s start_position, Tile *tilemap, ivec2s spawn) {
    Chunk chunk;
    chunk.tilemap    = malloc(CHUNK_SIZE_X * CHUNK_SIZE_Y * sizeof(*chunk.tilemap));
    chunk.spawn      = spawn;
    chunk.position   = (vec4s){start_position.x, start_position.y, start_position.x + (TILE_SIZE * CHUNK_SIZE_X), start_position.y + (TILE_SIZE * CHUNK_SIZE_Y) };

    memcpy(chunk.tilemap, tilemap, CHUNK_SIZE_X * CHUNK_SIZE_Y * sizeof(*chunk.tilemap));

    array_list_append(_chunk_list, &chunk);
    return _chunk_list->len - 1;
}

void chunk_render(void) {
    // SUGGEST: I think we can just give chunk a texture name and get it from here.
    struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_BASIC);

    Chunk *chunk = global.ChunkState.chunk;
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
        for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
            Tile tile = chunk->tilemap[CHUNK_SIZE_X * y + x]; 

             // convert to texture coordinate from uvs
            u32 row    = tile.uv / spritesheet->rows;
            u32 col    = tile.uv % spritesheet->cols;
            f32 celly  = spritesheet->stride / spritesheet->size.x; 
            f32 cellx  = spritesheet->stride / spritesheet->size.y;

            f32 tex_coord[4] = {cellx * col, cellx * col + cellx, celly * row, celly * row + celly};
            vec3s position   = {chunk->position.x + x * TILE_SIZE, chunk->position.y + y * TILE_SIZE, 0.0};
            batch_render_append_quad_texture(_chunk_batch, position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
        }
    }

    // render and clear batch
    batch_render_render(_chunk_batch);
    _chunk_batch->quad_count = 0;
}
