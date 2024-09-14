#ifndef PREFAB_H
#define PREFAB_H
#include "../gfx/spritesheet.h"

typedef struct {
    struct Spritesheet *spritesheet;
    f32 tex_coord[4];
    vec4s color;
    vec2s size;
} Prefab;

void prefab_init(void);
void prefab_destroy(void);
void prefab_create(char *name, struct Spritesheet *spritesheet, vec4s color, vec2s size, vec4s grid_coord);
#endif
