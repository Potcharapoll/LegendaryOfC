#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef struct {
    ivec2s coord;
    u32 chunkId;
} ChunkTeleporter;

typedef struct {
    ivec2s coord;
    u32 dialogId;
} ChunkDialog;

typedef struct {
    vec4s  position; // {startX, startY, endX, endY}
    ivec2s spawn; 
    
    u32 *uv;
    u32 *collision;

    u32 dialog_count;
    ChunkDialog *dialog;

    u32 teleporter_count;
    ChunkTeleporter *teleporter;
} Chunk;

// Explaination: Chunk contains tilemap information that uses to render the map
//               also fixed size at 30x24 and 32x32 tile size

void chunk_init(void);
void chunk_destroy(void);
void chunk_prepare(void);
void chunk_render(void);
u64 chunk_load_from_file(char *path);
Chunk* chunk_get(u64 chunk_id);
#endif
