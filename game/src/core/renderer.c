#include <string.h>
#include "renderer.h"
#include "asset_manager.h"
#include "camera.h"
#include "../util/log.h"
#include "../defs.h"

static int texture_slot[8] = {0,1,2,3,4,5,6,7};

static AssetManager *_asset_manager = NULL;
static Camera *_camera              = NULL;
static Batch **_batches             = NULL;


static void batch_init(Batch **batch) {
    *batch                = malloc(sizeof(Batch));
    ASSERT_MSG(batch != NULL, "Failed to allocate memory for render batch");

    (*batch)->vao           = vao_create();
    (*batch)->vbo           = vbo_create(GL_ARRAY_BUFFER, true);
    (*batch)->ebo           = vbo_create(GL_ELEMENT_ARRAY_BUFFER, false);
    (*batch)->shader        = *(Shader*)asset_manager_get_shader(_asset_manager, "default_shader"); 
    (*batch)->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));
    (*batch)->indices       = malloc(MAX_INDICES_PER_BATCH * sizeof(u32));
    (*batch)->hasRoom       = true;
    (*batch)->texture_count = 0;
    (*batch)->count         = 0;

    memset((*batch)->texture, 0, sizeof(texture_t)*8);
    ASSERT_MSG((*batch)->vertices != NULL, "Failed to allocate memory for vertices");
    ASSERT_MSG((*batch)->indices != NULL, "Failed to allocate memory for indices");

    for (int i = 0; i < MAX_QUAD_PER_BATCH; i++) {
        u32 offset = i * 4;
        u32 idx    = i * 6;

        (*batch)->indices[idx+0] = offset + 0;
        (*batch)->indices[idx+1] = offset + 1;
        (*batch)->indices[idx+2] = offset + 3;
        (*batch)->indices[idx+3] = offset + 0;
        (*batch)->indices[idx+4] = offset + 2;
        (*batch)->indices[idx+5] = offset + 3;
    }

    vao_bind((*batch)->vao);
    vbo_bind((*batch)->vbo);
    vbo_data((*batch)->vbo, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex), NULL);
    vao_attr(0, 3, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, position));
    vao_attr(1, 4, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, color));
    vao_attr(2, 2, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, tex_coord));
    vao_attr(3, 1, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, tex_slot));

    vbo_bind((*batch)->ebo);
    vbo_data((*batch)->ebo, MAX_INDICES_PER_BATCH * sizeof(u32), (*batch)->indices);
    free((*batch)->indices); (*batch)->indices = NULL;

    vao_unbind();
    vbo_unbind((*batch)->vbo);
    vbo_unbind((*batch)->ebo);
} 

static void _append_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color, vec2s uvs[4], u32 tex_slot) {
    vec2s tex_coord[4] = { {0,0}, {1,0}, {0,1}, {1,1} };

    if (uvs != NULL) {
        memcpy(tex_coord, uvs, sizeof(vec2s)*4);
    }

    size_t offset = _batches[layer]->count * 4;
    _batches[layer]->vertices[offset] = (BatchVertex) {
        .position  = position,
        .color     = color,
        .tex_coord = tex_coord[0],
        .tex_slot  = tex_slot,
    };
    _batches[layer]->vertices[offset+1] = (BatchVertex) {
        .position  = {position.x + size.x, position.y, position.z},
        .color     = color,
        .tex_coord = tex_coord[1],
        .tex_slot  = tex_slot,
    };
    _batches[layer]->vertices[offset+2] = (BatchVertex) {
        .position  = {position.x, position.y + size.y, position.z},
        .color     = color,
        .tex_coord = tex_coord[2],
        .tex_slot  = tex_slot,
    };
    _batches[layer]->vertices[offset+3] = (BatchVertex) {
        .position  = {position.x + size.x, position.y + size.y, position.z},
        .color     = color,
        .tex_coord = tex_coord[3],
        .tex_slot  = tex_slot,
    };

    _batches[layer]->count++;
}

// push texture to all layer of render batch
static f32 renderer_push_texture(Batch *batch, texture_t texture) {
    for (size_t i = 0; i < batch->texture_count; i++) {
        if (batch->texture[i].handle == texture.handle) {
            return i;
        }
    }
    batch->texture[batch->texture_count++] = texture;

    return batch->texture_count - 1;
}

void renderer_init(Camera *camera, AssetManager *assetmanager) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);

    _batches = malloc(LAYER_COUNT * sizeof(Batch*));
    ASSERT_MSG(_batches != NULL, "Failed to allocate memory for batches");

    _asset_manager = assetmanager;
    _camera        = camera;
    asset_manager_push_shader(_asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");

    for (int i = 0; i < LAYER_COUNT; i++) {
        _batches[i] = NULL;
    }
}

void renderer_destroy(void) {
    for (int i = 0; i < LAYER_COUNT; i++) { 
        if (_batches[i] != NULL) {
            vao_destroy(_batches[i]->vao);
            vbo_destroy(_batches[i]->vbo);
            vbo_destroy(_batches[i]->ebo);
            shader_destroy(_batches[i]->shader);
            for (u32 j = 0; j < _batches[i]->texture_count; j++) texture_destroy(_batches[i]->texture[j]);

            free(_batches[i]->vertices);
            free(_batches[i]);
        }
    }
    free(_batches);
    LOG_DEBUG("Renderer destroyed");
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    for (u32 i = 0; i < LAYER_COUNT; i++) {
        if (_batches[i] != NULL) _batches[i]->count = 0;
    }
}

void renderer_render_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color) {
    if (_batches[layer] == NULL) {
        batch_init(&_batches[layer]);
    }

    if (!_batches[layer]->hasRoom) {
        LOG_ERROR("Layer %d is full", layer);
        return;
    }
    
    _append_quad(layer, size, position, color, NULL, -1);
}

void renderer_render_sprite_sheet(RenderLayer layer, spritesheet_t *spritesheet, vec2s size, vec3s position, u32 row, u32 col) {
    if (_batches[layer] == NULL) {
        batch_init(&_batches[layer]);
    }

    if (!_batches[layer]->hasRoom) {
        LOG_ERROR("Layer %d is full", layer);
        return;
    }

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

void renderer_render_sprite_sheet_from(RenderLayer layer, spritesheet_t *spritesheet, vec2s size, vec3s position, 
        u32 start_row, u32 start_col, u32 end_row, u32 end_col) {
    if (_batches[layer] == NULL) {
        batch_init(&_batches[layer]);
    }

    if (!_batches[layer]->hasRoom) {
        LOG_ERROR("Layer %d is full", layer);
        return;
    }

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
    spritesheet_t *texture = asset_manager_get_spritesheet(_asset_manager, TEXTURE_CHUNK);
    for (u32 i = 0; i < (chunk->cols * chunk->rows); i++) {
        u32 texture_row = chunk->tile_texture_uv[i] / texture->rows;
        u32 texture_col = chunk->tile_texture_uv[i] % texture->cols;

        renderer_render_sprite_sheet(TERRAIN_LAYER, texture, chunk->tiles[i].size, chunk->tiles[i].position, texture_row, texture_col); 
    }

    for (u32 i = 0; i < chunk->structures->len; i++) {
        Structure *s = array_list_get(chunk->structures, i);
        renderer_render_sprite_sheet_from(STRUCTURE_LAYER, texture, s->size, s->position, s->row, s->col, s->row_width, s->col_width);
    }

    spritesheet_t *npc_texture = asset_manager_get_spritesheet(_asset_manager, TEXTURE_NPC);
    for (u32 i = 0; i < chunk->npcs->len; i++) {
        NPC *npc = array_list_get(chunk->npcs, i);
        renderer_render_sprite_sheet(STRUCTURE_LAYER, npc_texture, npc->size, npc->position, npc->row, npc->col);
    }
    
}

void renderer_render(void) {
    for (u32 i = 0; i < LAYER_COUNT; i++) {
        if (_batches[i] == NULL || _batches[i]->count <= 0) continue;

        vbo_bind(_batches[i]->vbo);
        vbo_subdata(_batches[i]->vbo, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex), _batches[i]->vertices);

        for (u32 j = 0; j < _batches[i]->texture_count; j++) { texture_bind(_batches[i]->texture[j], j); }
        shader_bind(_batches[i]->shader);
        ViewProj view_proj = get_view_proj(_camera);
        shader_uniform_viewproj(_batches[i]->shader, view_proj);
        shader_uniform_int_array(_batches[i]->shader, "tex", 8, texture_slot);

        vao_bind(_batches[i]->vao);
        glDrawElements(GL_TRIANGLES, (_batches[i]->count * 6), GL_UNSIGNED_INT, NULL);

        vao_unbind();
        vbo_unbind(_batches[i]->vbo);
        shader_unbind();
        texture_unbind();
    }
}

