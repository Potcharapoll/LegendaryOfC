#ifndef RENDERER_H
#define RENDERER_H
#include "types.h"
#include <cglm/struct.h>

void renderer_init(void);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_render(void);
void renderer_render_quad(vec3s position, vec2s size, vec4s color);
void renderer_render_quad_texture(vec3s position, vec2s size, vec4s color, u32 texture);
void renderer_render_triangle(vec3s position, vec2s size, vec4s color);
void renderer_render_text(vec3s position, vec4s color, char *text);
void renderer_render_dialog_text_animation(vec3s position, vec4s color, char *text);
#endif
