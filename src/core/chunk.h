#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include "../util/array_list.h"

#include <cglm/types-struct.h>

// Size 16x16 fixed
// For res/tilesets/test.png (bl -> tr)
enum TileType {
    TILE_UP,
    TILE_DOWN,
    TILE_RIGHT,

    TILE_LEFT,
    TILE_DIRT,
    TILE_LIGHTER_DIRT,

    TILE_LIGHT_GRASS,
    TILE_PATH,
    TILE_GRASS,
};

typedef struct {
    // Tile type and UV will be the same
    u32 uvs;

    u32 body_id;
    u32 action_id;
} Tile;

typedef struct {
    ivec2s size;
    ivec2s grid_coord;
    u32 row_width;
    u32 col_width;
} Structure;

typedef struct {
    ivec2s size;
    ivec2s grid_coord;
} NPC;

typedef struct {
    vec2s start_position;
    vec2s end_position;

    Tile *tilemap;
    ivec2s grid_size;

    // later
    array_list *structures;
    array_list *npcs;
} Chunk;

void chunk_init(void);
void chunk_destroy(void);
void chunk_render(void);
void chunk_update(void);

Chunk* chunk_create(ivec2s grid_size, vec2s start_position, Tile *tilemap);

void chunk_push_npc(Chunk *chunk, ivec2s grid_coord, ivec2s uv);
void chunk_push_structure(Chunk *chunk, vec2s size, ivec2s grid_coord, ivec2s start_uv, ivec2s end_uv);

ivec2s chunk_get_grid_position(Chunk *chunk, vec3s position);
#endif
