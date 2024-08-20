#include <string.h>
#include "renderer.h"
#include "../util/log.h"
#include "../defs.h"
#include "../global.h"

static int texture_slot[8] = {0,1,2,3,4,5,6,7};
static Batch **_batches = NULL;

static Batch* batch_init(void) {
    Batch *batch = malloc(sizeof(Batch));
    ASSERT_MSG(batch != NULL, "Failed to allocate memory for render batch");

    batch->vertices = malloc(MAX_BATCH_VERTICES * sizeof(Vertex));
    batch->indices  = malloc(MAX_BATCH_INDICES * sizeof(u32));

    ASSERT_MSG(batch->vertices != NULL, "Failed to allocate memory for vertices");
    ASSERT_MSG(batch->indices != NULL, "Failed to allocate memory for indices");

    for (u32 i = 0; i < MAX_BATCH_QUAD; i++) {
        u32 offset = i * 4;
        u32 idx    = i * 6;

        batch->indices[idx+0] = offset + 0;
        batch->indices[idx+1] = offset + 1;
        batch->indices[idx+2] = offset + 3;
        batch->indices[idx+3] = offset + 0;
        batch->indices[idx+4] = offset + 2;
        batch->indices[idx+5] = offset + 3;
    }

    GL_TRY(glGenVertexArrays(1, &batch->vao));
    GL_TRY(glBindVertexArray(batch->vao));



    GL_TRY(glGenBuffers(1, &batch->vbo));
    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, batch->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW));

    GL_TRY(glGenBuffers(1, &batch->ebo));
    GL_TRY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->ebo));
    GL_TRY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_BATCH_INDICES * sizeof(u32), batch->indices, GL_STATIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)));
    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color)));
    GL_TRY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord)));
    GL_TRY(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_slot)));

    GL_TRY(glEnableVertexAttribArray(0));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glEnableVertexAttribArray(2));
    GL_TRY(glEnableVertexAttribArray(3));

    {
        struct Shader *_shader = asset_manager_get_shader(global.asset_manager, "default_shader");  
        ASSERT_MSG(_shader != NULL, "Shader not found");

        batch->shader = *_shader;
    }

    batch->texture_count = 0;
    batch->quad_count    = 0;
    memset(batch->texture, 0, sizeof(struct Texture)*8);

    free(batch->indices); batch->indices = NULL;

    GL_TRY(glBindVertexArray(0));
    return batch;
} 

static void batch_destroy(Batch *batch) {
    GL_TRY(glDeleteVertexArrays(1, &batch->vao));
    GL_TRY(glDeleteBuffers(1, &batch->vbo));
    GL_TRY(glDeleteBuffers(1, &batch->ebo));

    for (u8 i = 0; i < batch->texture_count; i++) {
        texture_destroy(batch->texture[i]);
    }

    free(batch->vertices);
    free(batch);
}

static f32 renderer_push_texture(Batch *batch, struct Texture texture) {
    for (size_t i = 0; i < batch->texture_count; i++) {
        if (batch->texture[i].handle == texture.handle) {
            return i;
        }
    }
    batch->texture[batch->texture_count++] = texture;

    return batch->texture_count - 1;
}

static void _append_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color, vec2s uvs[4], u32 tex_slot) {
    vec2s tex_coord[4] = { {0,0}, {1,0}, {0,1}, {1,1} };

    if (uvs != NULL) {
        memcpy(tex_coord, uvs, sizeof(vec2s)*4);
    }

    size_t offset = _batches[layer]->quad_count * 4;
    _batches[layer]->vertices[offset] = (Vertex) {
        .position  = position,
        .color     = color,
        .tex_coord = tex_coord[0],
        .tex_slot  = tex_slot,
    };
    _batches[layer]->vertices[offset+1] = (Vertex) {
        .position  = {position.x + size.x, position.y, position.z},
        .color     = color,
        .tex_coord = tex_coord[1],
        .tex_slot  = tex_slot,
    };
    _batches[layer]->vertices[offset+2] = (Vertex) {
        .position  = {position.x, position.y + size.y, position.z},
        .color     = color,
        .tex_coord = tex_coord[2],
        .tex_slot  = tex_slot,
    };
    _batches[layer]->vertices[offset+3] = (Vertex) {
        .position  = {position.x + size.x, position.y + size.y, position.z},
        .color     = color,
        .tex_coord = tex_coord[3],
        .tex_slot  = tex_slot,
    };

    _batches[layer]->quad_count++;
}

void renderer_init(void) {
    asset_manager_push_shader(global.asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");

    _batches = malloc(LAYER_COUNT * sizeof(Batch *));
    for (int i = 0; i < LAYER_COUNT; i++) { _batches[i] = NULL; }
}

void renderer_destroy(void) {
    for (int i = 0; i < LAYER_COUNT; i++) {
        if (_batches[i]) batch_destroy(_batches[i]);
    }
    free(_batches);

    LOG_DEBUG("Renderer destroyed");
}

void renderer_prepare(void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    for (int i = 0; i < LAYER_COUNT; i++) {
        if (_batches[i]) _batches[i]->quad_count = 0;
    }
}

void renderer_render_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color) {
    if (_batches[layer] == NULL) { _batches[layer] = batch_init(); }

    _append_quad(layer, size, position, color, NULL, -1);
}

void renderer_render_sprite_sheet(RenderLayer layer, struct Spritesheet *spritesheet, vec2s size, vec3s position, u32 row, u32 col) {
    if (_batches[layer] == NULL) { _batches[layer] = batch_init(); }

    f32 slot = renderer_push_texture(_batches[layer], spritesheet->texture);
    f32 n_w  = 1.0f / spritesheet->cols;
    f32 n_h  = 1.0f / spritesheet->rows;

    f32 s_x = n_w * col;
    f32 s_y = n_h * row;

    vec2s tex_coord[4];
    tex_coord[0] = (vec2s){s_x    , s_y    };
    tex_coord[1] = (vec2s){s_x+n_w, s_y    };
    tex_coord[2] = (vec2s){s_x    , s_y+n_h};
    tex_coord[3] = (vec2s){s_x+n_w, s_y+n_h};

    _append_quad(layer, size, position, WHITE, tex_coord, slot);
}

void renderer_render_sprite_sheet_from(RenderLayer layer, struct Spritesheet *spritesheet, vec2s size, vec3s position, 
        u32 start_row, u32 start_col, u32 end_row, u32 end_col) {
    if (_batches[layer] == NULL) { _batches[layer] = batch_init(); }

    f32 slot = renderer_push_texture(_batches[layer], spritesheet->texture);
    f32 n_w  = 1.0f / spritesheet->cols;
    f32 n_h  = 1.0f / spritesheet->rows;

    f32 s_x = n_w * start_col;
    f32 s_y = n_h * start_row;
    f32 e_x = n_w * end_col;
    f32 e_y = n_h * end_row;

    vec2s tex_coord[4];
    tex_coord[0] = (vec2s){s_x    , s_y    };
    tex_coord[1] = (vec2s){s_x+e_x, s_y    };
    tex_coord[2] = (vec2s){s_x    , s_y+e_y};
    tex_coord[3] = (vec2s){s_x+e_x, s_y+e_y};

    _append_quad(layer, size, position, WHITE, tex_coord, slot);
}

// fix later
void renderer_render_chunk(Chunk *chunk) {
    struct Spritesheet *texture = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_CHUNK);

    for (u32 i = 0; i < (chunk->cols * chunk->rows); i++) {
        u32 texture_row = chunk->tile_texture_uv[i] / texture->rows;
        u32 texture_col = chunk->tile_texture_uv[i] % texture->cols;

        renderer_render_sprite_sheet(LAYER_TILEMAP, texture, chunk->tiles[i].size, chunk->tiles[i].position, texture_row, texture_col); 
    }

    for (u32 i = 0; i < chunk->structures->len; i++) {
        Structure *s = array_list_get(chunk->structures, i);
        renderer_render_sprite_sheet_from(LAYER_STRUCTURE, texture, s->size, s->position, s->row, s->col, s->row_width, s->col_width);
    }

    struct Spritesheet *npc_texture = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_NPC);
    for (u32 i = 0; i < chunk->npcs->len; i++) {
        NPC *npc = array_list_get(chunk->npcs, i);
        renderer_render_sprite_sheet(LAYER_STRUCTURE, npc_texture, npc->size, npc->position, npc->row, npc->col);
    }
}

void renderer_render(void) {
    for (u32 i = 0; i < LAYER_COUNT; i++) {
        if (_batches[i] == NULL || _batches[i]->quad_count <= 0) continue;

        GL_TRY(glBindVertexArray(_batches[i]->vao));

        GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, _batches[i]->vbo));
        GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_BATCH_VERTICES * sizeof(Vertex), _batches[i]->vertices));

        for (u32 j = 0; j < _batches[i]->texture_count; j++) { 
            texture_bind(_batches[i]->texture[j], j); 
        }

        GL_TRY(shader_bind(_batches[i]->shader));

        struct ViewProj view_proj = get_view_proj(global.camera);
        GL_TRY(shader_uniform_viewproj(_batches[i]->shader, view_proj));
        GL_TRY(shader_uniform_int_array(_batches[i]->shader, "tex", 8, texture_slot));

        GL_TRY(glDrawElements(GL_TRIANGLES, (_batches[i]->quad_count * 6), GL_UNSIGNED_INT, NULL));

        GL_TRY(vao_unbind());
        GL_TRY(shader_unbind());
        GL_TRY(texture_unbind());
    }
}
