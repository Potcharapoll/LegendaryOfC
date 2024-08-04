#ifndef TILESET_H
#define TILESET_H
#include "texture.h"
#include "../util/types.h"

typedef struct {
    texture_t texture;
    u32 rows, cols;
    u32 tile_count;
    u32 stride;
} tileset_t;

tileset_t tileset_load(char *path, u32 stride);
void tileset_destroy(tileset_t self);
#endif
