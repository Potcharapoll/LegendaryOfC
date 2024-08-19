#ifndef RENDERER_H
#define RENDERER_H
#define MAX_BATCH_QUAD     10000
#define MAX_BATCH_VERTICES 40000
#define MAX_BATCH_INDICES  60000
#include "../gfx/vao.h"
#include "../gfx/vbo.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "dialog.h"
#include "chunk.h"

typedef struct {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32   tex_slot;
}Vertex;

typedef struct {
    u32 vao;
    u32 vbo;
    u32 ebo;
    struct Shader shader;

    Vertex *vertices;
    u32 *indices;

    struct Texture texture[8];
    u32 texture_count;

    u32 quad_count;
}Batch;

typedef enum {
    LAYER_TILEMAP,
    LAYER_STRUCTURE,
    LAYER_PLAYER,

    LAYER_COUNT
} RenderLayer;

// we will render accord to the number of z axis

void renderer_init(void);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_render(void);
void renderer_clean(void);

void renderer_render_chunk(Chunk *chunk);
#endif
