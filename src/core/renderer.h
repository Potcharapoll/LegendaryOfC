#ifndef RENDERER_H
#define RENDERER_H
#include "physics.h"
#include <cglm/struct.h>

typedef enum {
    CHUNK_SPAWN   = 0,
    CHUNK_VILLAGE = 1,

    CHUNK_LAST
} Chunks;

void renderer_init(void);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_render(void);
void renderer_clean(void);

void renderer_append_prefab(ivec2s coord, char *prefab_name);
void renderer_append_line_segment(vec2s a, vec2s b, vec4s color);
void renderer_append_quad_line(vec2s position, vec2s size, vec4s color);
void renderer_append_aabb(AABB aabb, vec4s color); 
#endif
