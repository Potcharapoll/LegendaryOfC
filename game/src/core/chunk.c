#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "chunk.h"
#include "../defs.h"

void chunk_init(Chunk **chunk, u32 rows, u32 cols, vec2s start_position, u32 *tile_texture_uv) {
    static u64 _id = 0;

    (*chunk) = malloc(sizeof(**chunk));
    assert(*chunk != NULL);

    (*chunk)->uid             = _id++;
    (*chunk)->rows            = rows;
    (*chunk)->cols            = cols;
    (*chunk)->start_position  = start_position;
    (*chunk)->tiles           = malloc(cols * rows * sizeof(Tile));
    (*chunk)->tile_texture_uv = tile_texture_uv;
    (*chunk)->structures      = array_list_init(sizeof(Structure), 0);
    (*chunk)->npcs            = array_list_init(sizeof(NPC), 0);

    f32 _x = start_position.x;
    f32 _y = start_position.y;

    for (u32 y = 0; y < rows; y++) {
        for (u32 x = 0; x < cols; x++) {
            if ((int)_x % (TILE_SIZE * cols) == 0) { _x = (*chunk)->start_position.x; }

            (*chunk)->tiles[y * cols + x].color    = WHITE;
            (*chunk)->tiles[y * cols + x].size     = DEFAULT_SCALE;
            (*chunk)->tiles[y * cols + x].position = (vec3s){_x, _y, 0};
            _x += TILE_SIZE;
        }
        _y += TILE_SIZE;
    }
    (*chunk)->end_position.x = _x;
    (*chunk)->end_position.y = _y;
}

void chunk_push_npc(Chunk *chunk, vec2s size, vec2s chunk_rc, u32 row, u32 col) {
    vec3s position = { .x = TILE_SIZE * chunk_rc.x, .y = TILE_SIZE * chunk_rc.y, 0 };
    NPC npc = { .size = size, .color = WHITE, .position = position, .row = row, .col = col };

    array_list_append(chunk->npcs, &npc);
}

void chunk_push_structure(Chunk *chunk, vec2s size, vec2s chunk_rc, u32 row, u32 col, u32 row_width, u32 col_width) {
    vec3s position = { .x = TILE_SIZE * chunk_rc.x, .y = TILE_SIZE * chunk_rc.y, 0 };
    Structure s    = { .size = size, .color = WHITE, .position = position, .row = row, .col = col, .row_width = row_width, .col_width = col_width };

    array_list_append(chunk->structures, &s);
}

void chunk_destroy(Chunk *chunk) {
    array_list_destroy(chunk->npcs);
    array_list_destroy(chunk->structures);
    free(chunk->tiles);
    free(chunk);
}

ivec2s chunk_get_row_col_position(Chunk *chunk, vec3s position) {
    return (ivec2s) { .x = (s32)floor((position.x - chunk->start_position.x) / TILE_SIZE), .y = (s32)floor((position.y - chunk->start_position.y) / TILE_SIZE)};
}
