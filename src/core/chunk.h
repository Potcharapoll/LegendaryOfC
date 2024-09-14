#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef struct {
    u32 uv;
    u32 static_body_id;
} Tile;

typedef struct {
    vec4s  position; // {startX, startY, endX, endY}
    ivec2s spawn; 
    Tile  *tilemap;
} Chunk;

// Explaination: Chunk contains tilemap information that uses to render the map
//               also fixed size at 30x24 and 32x32 tile size

void chunk_init(void);
void chunk_destroy(void);
void chunk_render(void);
void chunk_update(void);
u64 chunk_load_from_file(char *path);
u64 chunk_create(vec2s start_position, Tile *tilemap, ivec2s spawn);
Chunk* chunk_get(u64 chunk_id);
#endif
