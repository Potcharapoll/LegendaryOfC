#include "chunk.h"

#include "../engine/logger.h"
#include "../defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FORMAT_IN_POSITION   "pos:[%f,%f]\n"
#define FORMAT_IN_BASE       "%hhu "
#define FORMAT_IN_UPPER      "%hhu "
#define FORMAT_IN_PREFAB     "coord:[%f,%f] %s\n"
#define FORMAT_IN_COLLIDER   "pos:[%f,%f] size:[%u,%u]\n"
#define FORMAT_IN_TELEPORTER "pos:[%f,%f] size:[%u,%u] target_coord:[%f,%f] chunkid:%hhu tag:%c\n"
#define FORMAT_IN_DIALOG     "pos:[%f,%f] size:[%f,%f] tag:%s\n"
#define FORMAT_IN_COUNT      "count:%u\n"

Chunk* chunk_load_from_file(char *path) {
  FILE *stream;
  Chunk* chunk = malloc(sizeof(*chunk));
  ASSERT(chunk != NULL, "Failed to allocate memory for chunk", __FILE__, __LINE__);

  stream = fopen(path, "rb");
  if (stream == NULL) { LOG_FETAL("Failed to load chunk from file at path \'%s\'", path); }

  chunk->render_info = malloc(sizeof(*chunk->render_info));
  ASSERT(chunk->render_info != NULL, "Failed to allocate memory for chunk->render_info", __FILE__, __LINE__);

  chunk->render_info->uv = malloc((CHUNK_SIZE_X * CHUNK_SIZE_Y * 2) * sizeof(*chunk->render_info->uv));
  ASSERT(chunk->render_info->uv != NULL, "Failed to allocate memory for chunk->uv", __FILE__, __LINE__);

  chunk->render_info->prefab = array_list_init(sizeof(ChunkPrefab), 0);
  chunk->dialog     = array_list_init(sizeof(ChunkDialog), 0);
  chunk->teleporter = array_list_init(sizeof(ChunkTeleporter), 0);
  chunk->collider   = array_list_init(sizeof(ChunkCollider), 0);

  char c;
  uint32_t size;

  char *keywords[] = {"position", "base", "upper", "prefab", "collider", "teleporter", "dialog"};
  char key[50];

  while ((c = fgetc(stream)) != EOF) {
    if (c == ' ' || c == '\n') continue;

    if (c == '[') {
      fscanf(stream, "%[a-z]]", key);
    }

    if (0 == strcmp(key, keywords[0])) {
      fgets(key, sizeof(key), stream);
      fscanf(stream, FORMAT_IN_POSITION, &chunk->render_info->position.x, &chunk->render_info->position.y);
      chunk->render_info->position.z = chunk->render_info->position.x + (TILE_SIZE * CHUNK_SIZE_X);
      chunk->render_info->position.w = chunk->render_info->position.y + (TILE_SIZE * CHUNK_SIZE_Y);

#ifdef CHUNK_PRINT_CONTENT
      fprintf(stdout, "%.2f %.2f %.2f %.2f\n", chunk->render_info->position.x, chunk->render_info->position.y, chunk->render_info->position.z, chunk->render_info->position.w);
#endif
      memset(key, 0, sizeof(key));
    }
    else if (0 == strcmp(key, keywords[1])) {
      fgets(key, sizeof(key), stream);
      for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
        for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
          fscanf(stream, FORMAT_IN_BASE, &chunk->render_info->uv[y * CHUNK_SIZE_X + x]);
        }
      }
      memset(key, 0, sizeof(key));
    }
    else if (0 == strcmp(key, keywords[2])) {
      u32 offset = CHUNK_SIZE_X * CHUNK_SIZE_Y;
      fgets(key, sizeof(key), stream);
      for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
        for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
          fscanf(stream, FORMAT_IN_UPPER, &chunk->render_info->uv[offset + y * CHUNK_SIZE_X + x]);
        }
      }
      memset(key, 0, sizeof(key));
    }
    else if (0 == strcmp(key, keywords[3])) {
      fgets(key, sizeof(key), stream);
      fscanf(stream, FORMAT_IN_COUNT, &size);

      ChunkPrefab prefab = {0};
      for (uint8_t i = 0; i < size; ++i) {
        fscanf(stream, FORMAT_IN_PREFAB, &prefab.coord.x, &prefab.coord.y, prefab.name);

#ifdef CHUNK_PRINT_CONTENT
        fprintf(stdout, FORMAT_IN_PREFAB, prefab.coord.x, prefab.coord.y, prefab.name);
#endif
        array_list_append(chunk->render_info->prefab, &prefab);
      }

      memset(key, 0, sizeof(key));
    }
    else if (0 == strcmp(key, keywords[4])) {
      fgets(key, sizeof(key), stream);
      fscanf(stream, FORMAT_IN_COUNT, &size);

      ChunkCollider collider = {0};
      for (uint8_t i = 0; i < size; ++i) {
        fscanf(stream, FORMAT_IN_COLLIDER, &collider.pos.x, &collider.pos.y, &collider.size.x, &collider.size.y);

#ifdef CHUNK_PRINT_CONTENT
        fprintf(stdout, FORMAT_IN_COLLIDER, collider.pos.x, collider.pos.y, collider.size.x, collider.size.y);
#endif
        array_list_append(chunk->collider, &collider);
      }

      memset(key, 0, sizeof(key));
    }
    else if (0 == strcmp(key, keywords[5])) {
      fgets(key, sizeof(key), stream);
      fscanf(stream, FORMAT_IN_COUNT, &size);

      ChunkTeleporter teleporter = {0};
      for (uint8_t i = 0; i < size; ++i) {
        fscanf(stream, FORMAT_IN_TELEPORTER, 
            &teleporter.pos.x, &teleporter.pos.y, 
            &teleporter.size.x, &teleporter.size.y, 
            &teleporter.target_coord.x, &teleporter.target_coord.y, 
            &teleporter.chunkId, &teleporter.tag);

#ifdef CHUNK_PRINT_CONTENT
        fprintf(stdout, FORMAT_IN_TELEPORTER, 
            teleporter.pos.x, teleporter.pos.y, 
            teleporter.size.x, teleporter.size.y, 
            teleporter.target_coord.x, teleporter.target_coord.y, 
            teleporter.chunkId, teleporter.tag);
#endif
        array_list_append(chunk->teleporter, &teleporter);
      }

      memset(key, 0, sizeof(key));
    }
    else if (0 == strcmp(key, keywords[6])) {
      fgets(key, sizeof(key), stream);
      fscanf(stream, FORMAT_IN_COUNT, &size);

      ChunkDialog dialog = {0};
      for (uint8_t i = 0; i < size; ++i) {
        fscanf(stream, FORMAT_IN_DIALOG, &dialog.pos.x, &dialog.pos.y, &dialog.size.x, &dialog.size.y, dialog.tag);

#ifdef CHUNK_PRINT_CONTENT
        fprintf(stdout, FORMAT_IN_DIALOG, dialog.pos.x, dialog.pos.y, dialog.size.x, dialog.size.y, dialog.tag);
#endif
        array_list_append(chunk->dialog, &dialog);
      }

      memset(key, 0, sizeof(key));
    }
  }
  fclose(stream);

  LOG_DEBUG("Chunk: Successfully to load chunk from file at path \'%s\'", path);
  return chunk;
}

void chunk_destroy(Chunk **self) {
  Chunk *tmp = *self;

  array_list_destroy(tmp->render_info->prefab);
  array_list_destroy(tmp->dialog);
  array_list_destroy(tmp->collider);
  array_list_destroy(tmp->teleporter);

  free(tmp->render_info->uv);
  free(tmp->render_info);
  free(tmp);

  *self = NULL;
}
