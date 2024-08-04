#ifndef RENDERER_H
#define RENDERER_H
#define MAX_ENTITY_PER_BATCH   10000
#define MAX_VERTICES_PER_BATCH 40000
#define MAX_INDICES_PER_BATCH  60000
#define MAX_BATCH_CAPACITY     32
#include "../gfx/vao.h"
#include "../gfx/vbo.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"

typedef struct {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32 tex_slot;
} BatchVertex;

typedef struct {
    struct VAO vao;
    struct VBO vbo, ebo;
    shader_t shader;
    texture_t texture[8];

    u32 count;
    u32 texture_count;
    BatchVertex *vertices;
    u32 *indices;
    b8 hasRoom;
}Batch;

typedef struct {
    size_t batch_len;
    Batch **batches;

    u64 total_entity;
} Renderer;

void renderer_init(Renderer **renderer);
void renderer_destroy(Renderer *renderer);
void renderer_prepare(void);
void renderer_clean(Renderer *renderer);
void renderer_append_quad(Renderer *renderer, vec2s size, vec3s position, vec4s color);
void renderer_push_texture(Renderer *renderer, texture_t texture);
void renderer_render(Renderer *renderer);
#endif
