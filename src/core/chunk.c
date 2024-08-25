#include "chunk.h"
#include "batch_render.h"

#include "../util/array_list.h"
#include "../global.h"
#include "../defs.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static struct BatchRender *_chunk_batch = NULL;
static array_list *_chunk_list = NULL;

void chunk_init(void) {
    _chunk_batch = batch_render_init();
    _chunk_list  = array_list_init(sizeof(Chunk), 4);
}

void chunk_destroy(void) {
    batch_render_destroy(_chunk_batch);

    for (u32 id = 0; id < _chunk_list->len; id++) {
        Chunk *chunk = array_list_get(_chunk_list, id);

        array_list_destroy(chunk->npcs);
        array_list_destroy(chunk->structures);
        free(chunk->tilemap);
    }

    array_list_destroy(_chunk_list);
}

Chunk* chunk_create(ivec2s grid_size, vec2s start_position, Tile *tilemap) {
    Chunk *chunk      = malloc(sizeof(*chunk));
    chunk->tilemap    = malloc(grid_size.x * grid_size.y * sizeof(*chunk->tilemap));
    chunk->grid_size  = grid_size;
    chunk->npcs       = array_list_init(sizeof(NPC), 0);
    chunk->structures = array_list_init(sizeof(Structure), 0);

    memcpy(chunk->tilemap, tilemap, grid_size.x * grid_size.y * sizeof(*chunk->tilemap));

    // Suggest: I think we can convert from this to use body instead (body_id)
    chunk->start_position = start_position;
    chunk->end_position   = (vec2s){start_position.x + (32*grid_size.x), start_position.y + (32*grid_size.y)};

    u32 _id = array_list_append(_chunk_list, chunk);

    return array_list_get(_chunk_list, _id);
}

void chunk_render(void) {
    // SUGGEST: I think we can just give chunk a texture name and get it from here.
    struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_BASIC);

    Chunk *chunk = global.ChunkState.chunk;
    for (s32 y = 0; y < chunk->grid_size.y; y++) {
        for (s32 x = 0; x < chunk->grid_size.x; x++) {
            Tile tile = chunk->tilemap[chunk->grid_size.x * y + x]; 

             // convert to texture coordinate from uvs
            u32 row    = floor((f32)tile.uvs / spritesheet->rows);
            u32 col    = tile.uvs % spritesheet->cols;
            f32 celly  = spritesheet->stride / spritesheet->size.x; 
            f32 cellx  = spritesheet->stride / spritesheet->size.y;

            f32 tex_coord[4] = {cellx * col, cellx * col + cellx, celly * row, celly * row + celly};
            vec3s position   = {chunk->start_position.x + x * TILE_SIZE, chunk->start_position.y + y * TILE_SIZE, 0.0};
            batch_render_append_quad_texture(_chunk_batch, position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
        }
    }

    // render and clear batch
    batch_render_render(_chunk_batch);
    _chunk_batch->quad_count = 0;
}

void chunk_update(void) {

}

ivec2s chunk_get_grid_position(Chunk *chunk, vec3s position) {
    return (ivec2s) { .x = (s32)floor((position.x - chunk->start_position.x) / TILE_SIZE), .y = (s32)floor((position.y - chunk->start_position.y) / TILE_SIZE)};
}
