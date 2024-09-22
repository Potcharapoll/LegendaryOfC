#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef struct {
    ivec2s teleport_coord;
    ivec2s target_coord;
    u8 chunkId;
    u32 body_id;
} ChunkTeleporter;

typedef struct {
    ivec2s coord;
    u8 dialogId;
    u32 body_id;
} ChunkDialog;

typedef struct {
    char name[50];
    ivec2s coord;
} ChunkPrefab;

typedef struct {
    vec4s  position; // {startX, startY, endX, endY}
    ivec2s spawn; 
    
    u16 *uv;
    u8 *collision;

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

    CHUNK_LAST              = 9
} Chunks;

// Explaination: Chunk contains tilemap information that uses to render the map
//               also fixed size at 30x24 and 32x32 tile size

Chunk* chunk_load_from_file(char *path);
void chunk_render(void);
#endif
