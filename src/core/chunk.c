#include "../global.h"
#include "../defs.h"
#include "../engine/logger.h"

#include "chunk.h"
#include "renderer.h"

#include <string.h>

#define FORMAT_IN_POSITION   "pos:[%f,%f]\n"
#define FORMAT_IN_BASE       "%hhu "
#define FORMAT_IN_UPPER      "%hhu "
#define FORMAT_IN_PREFAB     "coord:[%f,%f] %s\n"
#define FORMAT_IN_COLLIDER   "pos:[%f,%f] size:[%u,%u]\n"
#define FORMAT_IN_TELEPORTER "pos:[%f,%f] size:[%u,%u] target_coord:[%f,%f] chunkid:%hhu\n"
#define FORMAT_IN_DIALOG     "coord:[%u,%u] size:[%u,%u] dialogid:%hhu\n"
#define FORMAT_IN_COUNT      "count:%u\n"

Chunk* chunk_load_from_file(char *path) {
    FILE *stream;
    Chunk* chunk = malloc(sizeof(*chunk));

    stream = fopen(path, "rb");
    if (stream == NULL) { LOG_FETAL("Failed to load chunk from file at path \'%s\'", path); }

    chunk->uv = malloc((CHUNK_SIZE_X * CHUNK_SIZE_Y * 2) * sizeof(*chunk->uv));
    ASSERT(chunk->uv != NULL, "Failed to allocate memory for chunk->uv", __FILE__, __LINE__);

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
            fscanf(stream, FORMAT_IN_POSITION, &chunk->position.x, &chunk->position.y);
            chunk->position.z = chunk->position.x + (TILE_SIZE * CHUNK_SIZE_X);
            chunk->position.w = chunk->position.y + (TILE_SIZE * CHUNK_SIZE_Y);
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[1])) {
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, FORMAT_IN_BASE, &chunk->uv[y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[2])) {
            u32 offset = CHUNK_SIZE_X * CHUNK_SIZE_Y;
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, FORMAT_IN_UPPER, &chunk->uv[offset + y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[3])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, FORMAT_IN_COUNT, &size);
            chunk->prefab = malloc(size * sizeof(*chunk->prefab));
            ASSERT(chunk->prefab != NULL, "Failed to allocate memory for chunk->prefab", __FILE__, __LINE__);
            chunk->prefab_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, FORMAT_IN_PREFAB, &chunk->prefab[i].coord.x, &chunk->prefab[i].coord.y, chunk->prefab[i].name);
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[4])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, FORMAT_IN_COUNT, &size);
            chunk->collider = malloc(size * sizeof(*chunk->collider));
            ASSERT(chunk->collider != NULL, "Failed to allocate memory for chunk->collider", __FILE__, __LINE__);
            chunk->collider_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, FORMAT_IN_COLLIDER, 
                    &chunk->collider[i].pos.x, &chunk->collider[i].pos.y,
                    &chunk->collider[i].size.x, &chunk->collider[i].size.y
                    );
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[5])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, FORMAT_IN_COUNT, &size);
            chunk->teleporter = malloc(size * sizeof(*chunk->teleporter));
            ASSERT(chunk->teleporter != NULL, "Failed to allocate memory for chunk->teleporter", __FILE__, __LINE__);
            chunk->teleporter_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, FORMAT_IN_TELEPORTER, 
                        &chunk->teleporter[i].pos.x, &chunk->teleporter[i].pos.y, 
                        &chunk->teleporter[i].size.x, &chunk->teleporter[i].size.y, 
                        &chunk->teleporter[i].target_coord.x, &chunk->teleporter[i].target_coord.y, 
                        &chunk->teleporter[i].chunkId);
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[6])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, FORMAT_IN_COUNT, &size);
            chunk->dialog = malloc(size * sizeof(*chunk->dialog));
            ASSERT(chunk->dialog != NULL, "Failed to allocate memory for chunk->dialog", __FILE__, __LINE__);
            chunk->dialog_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, FORMAT_IN_DIALOG, 
                        &chunk->dialog[i].coord.x, &chunk->dialog[i].coord.y, 
                        &chunk->dialog[i].size.x, &chunk->dialog[i].size.y, 
                        &chunk->dialog[i].dialogId);
            }
            memset(key, 0, sizeof(key));
        }
    }
    fclose(stream);

    LOG_DEBUG("Chunk: Successfully to load chunk from file at path \'%s\'", path);
    return chunk;
}

void chunk_render(void) {
    Chunk *chunk = global.ChunkState.chunk;
    struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TILE);

    // base layer
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
        for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
            u32 uv = chunk->uv[CHUNK_SIZE_X * y + x]; 
            
            if (uv == (u32)-1) { continue; }

            u32 row    = uv / spritesheet->cols;
            u32 col    = uv % spritesheet->cols;
            f32 cellx  = spritesheet->stride / spritesheet->size.x;
            f32 celly  = spritesheet->stride / spritesheet->size.y; 

            f32 tex_coord[4] = {
                (cellx * col), 
                (cellx * col) + cellx, 
                (celly * row), 
                (celly * row) + celly
            };
            vec3s position   = {chunk->position.x + x * TILE_SIZE, chunk->position.y + y * TILE_SIZE, 0.0};
            renderer_append_quad_texture(LAYER_BASE, position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
        }
    }

    // base upper layer
    u32 offset = CHUNK_SIZE_Y * CHUNK_SIZE_X;
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
        for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
            u32 uv = chunk->uv[offset + CHUNK_SIZE_X * y + x]; 

            if (uv == 0) continue;

            u32 row    = uv / spritesheet->cols;
            u32 col    = uv % spritesheet->cols;
            f32 cellx  = spritesheet->stride / spritesheet->size.x;
            f32 celly  = spritesheet->stride / spritesheet->size.y; 

            f32 tex_coord[4] = {
                (cellx * col), 
                (cellx * col) + cellx, 
                (celly * row), 
                (celly * row) + celly
            };
            vec3s position   = {chunk->position.x + x * TILE_SIZE, chunk->position.y + y * TILE_SIZE, 0.0};
            renderer_append_quad_texture(LAYER_BASE_UPPER, position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
        }
    }

    // prefab
    for (u8 i = 0; i < chunk->prefab_count; ++i) {
        renderer_append_prefab(LAYER_STRUCTURE, chunk->prefab[i].coord, chunk->prefab[i].name);
    }
}
