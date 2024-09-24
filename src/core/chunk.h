#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef struct {
    vec2s  pos;
    vec2s  target_coord;
    ivec2s size;

    u8  chunkId;
    u32 body_id;
} ChunkTeleporter;

typedef struct {
    ivec2s coord;
    ivec2s size;
    u8 dialogId;
    u32 body_id;
} ChunkDialog;

typedef struct {
    char name[50];
    ivec2s coord;
} ChunkPrefab;

typedef struct {
    vec2s  pos;
    ivec2s size;
} ChunkCollider;

typedef struct {
    vec4s  position; // {startX, startY, endX, endY}
    u8    *uv;

    u8 collider_count;
    ChunkCollider *collider;

    u8 prefab_count;
    ChunkPrefab *prefab;

    u8 dialog_count;
    ChunkDialog *dialog;

    u8 teleporter_count;
    ChunkTeleporter *teleporter;
} Chunk;

typedef enum {
    CHUNK_SPAWN             = 0,
    CHUNK_VILLAGE_ENTRANCE  = 1,
    CHUNK_VILLAGE_LEFT      = 2,
    CHUNK_VILLAGE_RIGHT     = 3,
    CHUNK_VILLAGE_TOP       = 4,
    CHUNK_VILLAGE_TOP_END   = 5,
    CHUNK_VILLAGE_TOP_LEFT  = 6,
    CHUNK_VILLAGE_TOP_RIGHT = 7,
    CHUNK_VILLAGE_TUNNEL    = 8,

    CHUNK_INSIDE_LIBRARY    = 9,
    CHUNK_INSIDE_RESTAURANT = 10,

    CHUNK_LAST = 10
} Chunks;

// Explaination: Chunk contains tilemap information that uses to render the map
//               also fixed size at 30x24 and 32x32 tile size

Chunk* chunk_load_from_file(char *path);
void chunk_render(void);
#endif
