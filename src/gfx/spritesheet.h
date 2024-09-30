#ifndef SPRITESHEET_H
#define SPRITESHEET_H
#include "../util/types.h"
#include "../gfx/texture.h"
#include <cglm/types-struct.h>

typedef struct Spritesheet {
    u32 count;
    ivec2s grid_size;
    ivec2s cell_size;
    vec2s size;

    struct Texture texture;
}Spritesheet;

Spritesheet spritesheet_load(char *path, u32 count, ivec2s grid_size, ivec2s cell_size);
void spritesheet_destroy(Spritesheet self);
#endif
