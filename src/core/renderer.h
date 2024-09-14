#ifndef RENDERER_H
#define RENDERER_H
#include "physics.h"
#include <cglm/struct.h>

typedef enum {
    CHUNK_SPAWN,
    CHUNK_VILLAGE,

    CHUNK_LAST
} Chunks;

void renderer_init(void);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_render(void);
void renderer_clean(void);

void renderer_append_line_segment(vec2s a, vec2s b, vec4s color);
void renderer_append_quad_line(vec2s position, vec2s size, vec4s color);
void renderer_append_aabb(AABB aabb, vec4s color); // use in line_batch 
void renderer_append_text(vec2s position, vec2s size, vec4s color, char *text);
void renderer_append_text_animation(vec2s position, vec2s size, vec4s color, char *text);
#endif
