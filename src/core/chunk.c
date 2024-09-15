#include "chunk.h"
#include "batch_render.h"

#include "../util/array_list.h"
#include "../global.h"
#include "../defs.h"
#include "physics.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static struct BatchRender *_chunk_batch = NULL;
static array_list *_chunk_list          = NULL;

void chunk_init(void) {
    _chunk_batch = batch_render_init();
    _chunk_list  = array_list_init(sizeof(Chunk), 4);
}

void chunk_destroy(void) {
    batch_render_destroy(_chunk_batch);

    for (u32 id = 0; id < _chunk_list->len; id++) {
        Chunk *chunk = chunk_get(id);
        free(chunk->uv);
        free(chunk->dialog);
        free(chunk->collision);
        free(chunk->teleporter);
    }

    array_list_destroy(_chunk_list);
}

Chunk* chunk_get(u64 chunk_id) {
    return array_list_get(_chunk_list, chunk_id);
}

u64 chunk_load_from_file(char *path) {
    FILE *stream;
    Chunk chunk;

    stream = fopen(path, "rb");
    if (stream == NULL) {
        fprintf(stderr, "Failed to open file \'path\'\n");
        exit(1);
    }

    chunk.collision = malloc(CHUNK_SIZE_X * CHUNK_SIZE_Y * sizeof(*chunk.collision));
    chunk.uv        = malloc(CHUNK_SIZE_X * CHUNK_SIZE_Y * sizeof(*chunk.uv));

    char c;
    uint32_t size;
     
    char *keywords[] = {"position", "spawn", "uv", "collision", "teleporter", "dialog"};
    char key[100];

    while ((c = fgetc(stream)) != EOF) {
        if (c == ' ' || c == '\n') continue;

        if (c == '[') {
            fscanf(stream, "%[a-z]]", key);
        }

        if (0 == strcmp(key, keywords[0])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%f %f", &chunk.position.x, &chunk.position.y);
            chunk.position.z = chunk.position.x + (TILE_SIZE * CHUNK_SIZE_X);
            chunk.position.w = chunk.position.y + (TILE_SIZE * CHUNK_SIZE_Y);
            memset(key, 0, sizeof(key));

        }
        else if (0 == strcmp(key, keywords[1])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%u %u", &chunk.spawn.x, &chunk.spawn.y);
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[2])) {
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, "%u ", &chunk.uv[y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[3])) {
            fgets(key, sizeof(key), stream);
            for (u8 y = 0; y < CHUNK_SIZE_Y; ++y) {
                for (u8 x = 0; x < CHUNK_SIZE_X; ++x) {
                    fscanf(stream, "%u ", &chunk.collision[y * CHUNK_SIZE_X + x]);
                }
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[4])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%u", &size);
            chunk.teleporter = malloc(size * sizeof(*chunk.teleporter));
            chunk.teleporter_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, "%u %u %u", &chunk.teleporter[i].coord.x, &chunk.teleporter[i].coord.y, &chunk.teleporter[i].chunkId);
            }
            memset(key, 0, sizeof(key));
        }
        else if (0 == strcmp(key, keywords[5])) {
            fgets(key, sizeof(key), stream);
            fscanf(stream, "%u", &size);
            chunk.dialog = malloc(size * sizeof(*chunk.dialog));
            chunk.dialog_count = size;

            for (uint8_t i = 0; i < size; ++i) {
                fscanf(stream, "%u %u %u", &chunk.dialog[i].coord.x, &chunk.dialog[i].coord.y, &chunk.dialog[i].dialogId);
            }
            memset(key, 0, sizeof(key));
        }
    }
    fclose(stream);

    array_list_append(_chunk_list, &chunk);
    return _chunk_list->len - 1;
}

void chunk_prepare(void) {
    Chunk *chunk = global.ChunkState.chunk;

    for (int i = 0; i < (CHUNK_SIZE_X*CHUNK_SIZE_Y); ++i) {
        if (chunk->collision[i] > 0) {
            u32 x = i % CHUNK_SIZE_X;
            u32 y = i / CHUNK_SIZE_X;

            if (chunk->collision[i] == 1) {
                physics_static_body_create((vec2s){TILE_SIZE * x, TILE_SIZE * y}, DEFAULT_SCALE, COLLISION_PLAYER, COLLISION_SOLID, NULL);
            }
        }
    }

    for (u32 i = 0; i < chunk->teleporter_count; ++i) {
        physics_static_body_create(
                (vec2s){TILE_SIZE * chunk->teleporter[i].coord.x, TILE_SIZE * chunk->teleporter[i].coord.y}, 
                DEFAULT_SCALE, COLLISION_PLAYER, COLLISION_TELEPORTER, NULL); 
    } 

    for (u32 i = 0; i < chunk->dialog_count; ++i) {
        physics_static_body_create(
                (vec2s){TILE_SIZE * chunk->dialog[i].coord.x, TILE_SIZE * chunk->dialog[i].coord.y}, 
                DEFAULT_SCALE, COLLISION_PLAYER, COLLISION_DIALOG, NULL); 
    } 
}

void chunk_render(void) {
    // SUGGEST: I think we can just give chunk a texture name and get it from here.
    struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_BASIC);

    Chunk *chunk = global.ChunkState.chunk;
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
        for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
            u32 uv = chunk->uv[CHUNK_SIZE_X * y + x]; 

             // convert to texture coordinate from uvs
            u32 row    = uv / spritesheet->rows;
            u32 col    = uv % spritesheet->cols;
            f32 celly  = spritesheet->stride / spritesheet->size.x; 
            f32 cellx  = spritesheet->stride / spritesheet->size.y;

            f32 tex_coord[4] = {cellx * col, cellx * col + cellx, celly * row, celly * row + celly};
            vec3s position   = {chunk->position.x + x * TILE_SIZE, chunk->position.y + y * TILE_SIZE, 0.0};
            batch_render_append_quad_texture(_chunk_batch, position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
        }
    }

    // render and clear batch
    batch_render_render(_chunk_batch);
    _chunk_batch->quad_count = 0;
}
