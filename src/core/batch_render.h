#ifndef BATCH_RENDER_H
#define BATCH_RENDER_H
#define MAX_QUAD_PER_BATCH 10000
#define MAX_VERTICES_PER_BATCH 40000
#define MAX_INDICES_PER_BATCH 60000

#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../util/types.h"

#include <cglm/struct.h>
#include <glad/glad.h>

struct Vertex {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32   tex_slot; // -1 for none, and 0 - 8 slot of texture
};

struct BatchRender {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    struct Shader shader;

    struct Texture textures[8];
    u32 texture_count;

    u32 quad_count;
    struct Vertex *vertices;
};

struct BatchRender *batch_render_init(void);
void batch_render_render(struct BatchRender *batch);
void batch_render_destroy(struct BatchRender *batch);
u32  batch_render_append_texture(struct BatchRender *batch, struct Texture texture);
u32 batch_render_get_texture_slot(struct BatchRender *batch, struct Texture texture);
void batch_render_append_quad(struct BatchRender *batch, vec3s position, vec2s size, vec4s color);
void batch_render_append_quad_texture(struct BatchRender *batch, vec3s position, vec2s size, vec4s color, struct Texture texture, f32 *tex_coord);
#endif
