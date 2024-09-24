#ifndef RENDERER_H
#define RENDERER_H
#define MAX_QUAD_PER_BATCH     10000
#define MAX_VERTICES_PER_BATCH 40000
#define MAX_INDICES_PER_BATCH  60000
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "physics.h"
#include "chunk.h"

#include <cglm/struct.h>

typedef enum {
    LAYER_BASE,
    LAYER_BASE_UPPER,
    LAYER_STRUCTURE,
    LAYER_PLAYER,
    LAYER_DIALOG,

    LAYER_LAST
} RenderLayer;

struct Vertex {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32   tex_slot; // -1 for none, and 0 - 8 slot of texture
};

struct LineVertex {
    vec2s position;
    vec4s color;
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

struct LineBatchRender {
    GLuint vao;
    GLuint vbo;
    struct Shader shader;

    u32 line_count;
    struct LineVertex *vertices;
};

void renderer_init(void);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_render(void);
void renderer_clean(void);

void renderer_append_prefab(RenderLayer layer, ivec2s coord, char *prefab_name);
void renderer_append_quad(RenderLayer layer, vec3s position, vec2s size, vec4s color);
void renderer_append_quad_texture(RenderLayer layer, vec3s position, vec2s size, vec4s color, struct Texture texture, f32 *tex_coord);

void renderer_set_chunk(Chunks chunkId, vec2s target_pos);
void renderer_reset_chunk(void);
void renderer_reload_chunk(void);

void renderer_append_line_segment(vec2s a, vec2s b, vec4s color);
void renderer_append_quad_line(vec2s position, vec2s size, vec4s color);
void renderer_append_aabb(AABB aabb, vec4s color); 

u32 renderer_batch_append_texture(RenderLayer layer, struct Texture texture);
#endif
