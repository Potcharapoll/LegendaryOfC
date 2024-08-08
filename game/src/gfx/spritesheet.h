#ifndef SPRITESHEET_H
#define SPRITESHEET_H
#include "../util/types.h"
#include "texture.h"
#include <cglm/types-struct.h>

typedef struct {
    u32 count;
    u32 rows;
    u32 cols;
    u32 stride;
    vec2s size;
    texture_t texture;
} spritesheet_t;

spritesheet_t spritesheet_load(char *path, u32 count, u32 rows, u32 cols, u32 stride);
void spritesheet_destroy(spritesheet_t self);
#endif
