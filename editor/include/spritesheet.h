#ifndef SPRITESHEET_H
#define SPRITESHEET_H
#include "texture.h"
#include "types.h"

typedef struct {
    struct Texture texture;
    u32 rows, cols;
    u32 sprite_count;
    ivec2s stride;
} spritesheet_t;

#endif
