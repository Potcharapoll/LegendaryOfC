#include "../global.h"
#include "../defs.h"

#include "chunk.h"
#include "renderer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Chunk* chunk_load_from_file(char *path) {
    FILE *stream;
    Chunk* chunk = malloc(sizeof(*chunk));

    stream = fopen(path, "rb");
    if (stream == NULL) {
        fprintf(stderr, "Failed to open file \'%s\'\n", path);
        exit(1);
    }

    chunk->collision = malloc(CHUNK_SIZE_X * CHUNK_SIZE_Y * sizeof(*chunk->collision));
    chunk->uv        = malloc((CHUNK_SIZE_X * CHUNK_SIZE_Y * 2) * sizeof(*chunk->uv));

    char c;
    uint32_t size;
     
    char *keywords[] = {"position", "spawn", "base", "upper", "prefab", "collision", "teleporter", "dialog"};
    char key[100];

    while ((c = fgetc(stream)) != EOF) {
        if (c == ' ' || c == '\n') continue;

        if (c == '[') {
            fscanf(stream, "%[a-z]]", key);
        }

        if (0 == strcmp(key, keywords[0])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%f %f", &chunk->position.x, &chunk->position.y);
            chunk->position.z = chunk->position.x + (TILE_SIZE * CHUNK_SIZE_X);
            chunk->position.w = chunk->position.y + (TILE_SIZE * CHUNK_SIZE_Y);
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[2])) {
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, "%hu ", &chunk->uv[y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[3])) {
            u32 offset = CHUNK_SIZE_X * CHUNK_SIZE_Y;
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, "%hu ", &chunk->uv[offset + y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[4])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%u", &size);
            chunk->prefab = malloc(size * sizeof(*chunk->prefab));
            chunk->prefab_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, "%u %u %s", &chunk->prefab[i].coord.x, &chunk->prefab[i].coord.y, chunk->prefab[i].name);
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[5])) {
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, "%hhu ", &chunk->collision[y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[6])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%u", &size);
            chunk->teleporter = malloc(size * sizeof(*chunk->teleporter));
            chunk->teleporter_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, "%u %u %u %u %hhu", 
                        &chunk->teleporter[i].teleport_coord.x, &chunk->teleporter[i].teleport_coord.y, 
                        &chunk->teleporter[i].target_coord.x, &chunk->teleporter[i].target_coord.y, 
                        &chunk->teleporter[i].chunkId);
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[7])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%u", &size);
            chunk->dialog = malloc(size * sizeof(*chunk->dialog));
            chunk->dialog_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, "%u %u %hhu", &chunk->dialog[i].coord.x, &chunk->dialog[i].coord.y, &chunk->dialog[i].dialogId);
            }
            memset(key, 0, sizeof(key));
        }
    }
    fclose(stream);

    return chunk;
}

void chunk_render(void) {
    Chunk *chunk = global.ChunkState.chunk;
    struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TILE);

    // base layer
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
        for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
            u32 uv = chunk->uv[CHUNK_SIZE_X * y + x]; 

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
