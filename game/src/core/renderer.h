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
#include "dialog.h"
#include "chunk.h"

typedef enum {
    TERRAIN_LAYER,
    STRUCTURE_LAYER,
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
void renderer_render_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color);
void renderer_render_sprite_sheet(RenderLayer layer, spritesheet_t *spritesheet, vec2s size, vec3s position, u32 row, u32 col);
void renderer_render_sprite_sheet_from(RenderLayer layer, spritesheet_t *spritesheet, vec2s size, vec3s position, u32 start_row, u32 start_col, u32 end_row, u32 end_col);
void renderer_render_chunk(Chunk *chunk);
void renderer_render_dialog(dialog_t *dialog, f32 dt);
#endif
