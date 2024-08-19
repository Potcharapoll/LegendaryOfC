#ifndef SPRITESHEET_H
#define SPRITESHEET_H
#include "../util/types.h"
#include "texture.h"
#include <cglm/types-struct.h>

struct Spritesheet {
    u32       count;
    u32       rows;
    u32       cols;
    u32       stride;
    vec2s     size;
    struct Texture texture;
};

struct Spritesheet spritesheet_load(char *path, u32 count, u32 rows, u32 cols, u32 stride);
void spritesheet_destroy(struct Spritesheet self);
#endif
