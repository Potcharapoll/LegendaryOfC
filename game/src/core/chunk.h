#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include "../util/array_list.h"
#include <cglm/types-struct.h>

typedef struct {
    vec2s size;
    vec3s position;
    vec4s color;

    u32 row;
    u32 col;
    u32 row_width;
    u32 col_width;
} Structure;

typedef struct {
    vec2s size;
    vec3s position;
    vec4s color;

    u32 row;
    u32 col;
} NPC;

typedef struct {
    vec2s size;
    vec3s position;
    vec4s color;

    u32 row;
    u32 col;
    u32 uv;
} Tile;

typedef struct {
    u64 uid;
    u32 rows;
    u32 cols;

    vec2s start_position;
    vec2s end_position;

    Tile *tiles;
    u32 *tile_texture_uv;

    array_list *structures;
    array_list *npcs;
} Chunk;

void chunk_init(Chunk **chunk, u32 rows, u32 cols, vec2s start_position, u32 *tile_texture_uv);
void chunk_push_npc(Chunk *chunk, vec2s size, vec2s chunk_rc, u32 row, u32 col);
void chunk_push_structure(Chunk *chunk, vec2s size, vec2s chunk_rc, u32 row, u32 col, u32 row_width, u32 col_width);
ivec2s chunk_get_row_col_position(Chunk *chunk, vec3s position);
void chunk_destroy(Chunk *chunk);
#endif
