#ifndef RENDERER_H
#define RENDERER_H
#define MAX_ENTITY_PER_BATCH 10000
#define MAX_VERTICES_PER_BATCH 40000
#define MAX_INDICES_PER_BATCH 60000
#include "vao.h"
#include "vbo.h"
#include "shader.h"
#include "texture.h"

typedef struct {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32 tex_slot;
} BatchVertex;

typedef struct {
    struct VAO vao;
    struct VBO vbo, ebo;
    struct Shader shader;
    struct Texture texture[8];

    u32 count;
    u32 texture_count;
    BatchVertex *vertices;
    u32 *indices;
}Batch;

void renderer_init(void);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_append_quad(vec2s size, vec3s position, vec4s color);
void renderer_render(void);
#endif
