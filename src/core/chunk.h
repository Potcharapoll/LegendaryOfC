#ifndef CHUNK_H
#define CHUNK_H
#include "../util/types.h"
#include "../util/array_list.h"
#include <cglm/types-struct.h>

typedef struct {
  vec2s  pos;
  vec2s  target_coord;
  ivec2s size;

  char tag;
  u8  chunkId;
  u32 body_id;
} ChunkTeleporter;

typedef struct {
  vec2s pos;
  vec2s size;

  char tag[50]; 
  u32 body_id;
} ChunkDialog;

typedef struct {
  char name[50];
  vec2s coord;
} ChunkPrefab;

typedef struct {
  vec2s  pos;
  ivec2s size;
} ChunkCollider;

typedef struct {
  u8 *uv;
  u8 prefab_count;

  vec4s position; 
  array_list *prefab;
}ChunkRenderInfo;

typedef struct {
  ChunkRenderInfo *render_info;
  array_list *collider;
  array_list *teleporter;
  array_list *dialog;
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
  CHUNK_INSIDE_CHURCH     = 11,
  CHUNK_INSIDE_FISH       = 12,
  CHUNK_INSIDE_OG_HOME    = 13,
  CHUNK_INSIDE_LJ_HOME    = 14,
  CHUNK_INSIDE_VC_HOME    = 15, 
} Chunks;
#define CHUNK_LAST (CHUNK_INSIDE_VC_HOME + 1)

Chunk* chunk_load_from_file(char *path);
void chunk_destroy(Chunk **self);
#endif
