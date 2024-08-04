#ifndef RENDERER_H
#define RENDERER_H
#define MAX_QUAD_PER_BATCH     10000
#define MAX_VERTICES_PER_BATCH 40000
#define MAX_INDICES_PER_BATCH  60000
#include "../gfx/vao.h"
#include "../gfx/vbo.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "asset_manager.h"
#include "camera.h"

typedef enum {
    TERRAIN_LAYER,
    BUILDING_LAYER,
    PLAYER_LAYER,

    LAYER_COUNT
}RenderLayer;

typedef struct {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32 tex_slot;
}BatchVertex;

typedef struct {
    VAO vao;
    VBO vbo, ebo;
    Shader shader;

    texture_t texture[8];
    u32 texture_count;

    BatchVertex *vertices;
    u32 *indices;
    u32 count;

    b8 hasRoom;
}Batch;

void renderer_init(Camera *camera, AssetManager *assetmanager);
void renderer_destroy(void);
void renderer_prepare(void);
void renderer_render(void);
void renderer_clean(void);
void renderer_append_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color);
void renderer_push_texture(texture_t texture);
#endif
