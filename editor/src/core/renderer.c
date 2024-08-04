#include <string.h>
#include "core/renderer.h"
#include "core/asset_manager.h"
#include "core/components.h"
#include "core/ecs.h"
#include "util/log.h"
#include "global.h"

static const int BATCHES_INITIAL_CAPACITY = 8;

static int texture_slot[8] = {0,1,2,3,4,5,6,7};
static ecs_world_t *_world = NULL;

static void renderer_update_vertices(Renderer *renderer, ecs_entity_t entity_id, vec2s stride, vec2s uv) {
    u32 batch_idx = entity_id / MAX_VERTICES_PER_BATCH;
    u32 idx       = entity_id % MAX_VERTICES_PER_BATCH;

    f32 coordX = stride.x * uv.x;
    f32 coordY = stride.y * uv.y;

    tileset_t *tileset = asset_manager_get_tileset(global.asset_manager, global.editor_state.tile_editor_state.selected_tileset);
    f32 tex_slot = 0;
    for (u32 i = 0; i < renderer->batches[batch_idx]->texture_count; i++) {
        if (tileset->texture.handle == renderer->batches[batch_idx]->texture[i].handle) {
            tex_slot = i;
        } 
    }
    
    idx *= 4;
    renderer->batches[batch_idx]->vertices[idx+0].tex_slot = tex_slot;
    renderer->batches[batch_idx]->vertices[idx+1].tex_slot = tex_slot;
    renderer->batches[batch_idx]->vertices[idx+2].tex_slot = tex_slot;
    renderer->batches[batch_idx]->vertices[idx+3].tex_slot = tex_slot;

    renderer->batches[batch_idx]->vertices[idx+0].tex_coord = (vec2s){coordX, coordY};
    renderer->batches[batch_idx]->vertices[idx+1].tex_coord = (vec2s){coordX + stride.x, coordY};
    renderer->batches[batch_idx]->vertices[idx+2].tex_coord = (vec2s){coordX, coordY + stride.y};
    renderer->batches[batch_idx]->vertices[idx+3].tex_coord = (vec2s){coordX + stride.x, coordY + stride.y};
}

static void _update_ecs_state(void) {
    ecs_query_t *query = ecs_query(_world);

    for (ecs_entity_t id = 0; id < query->len; id++) {
        const Updatable *state = ecs_get(_world,id,UPDATABLE_COMPONENT);

        ecs_flag_t flag = state->update_flag;
        if (flag & UPDATE_TEXTURE) {
            const Sprite *spr = ecs_get(_world, id, SPRITE_COMPONENT);
            tileset_t *tileset = asset_manager_get_tileset(global.asset_manager, global.editor_state.tile_editor_state.selected_tileset);
            ecs_set(_world, id, SPRITE_COMPONENT, &(Sprite){tileset->texture.handle, spr->spriteWidth, spr->spriteHeight});
            renderer_update_vertices(global.renderer, id, global.editor_state.tile_editor_state.tile_stride, global.editor_state.tile_editor_state.selected_tile_idx);

            ecs_set(_world, id, UPDATABLE_COMPONENT, &(Updatable){0});
        }
    }
}

static Batch* batch_init(void) {
    Batch *batch = malloc(sizeof(*batch));
    assert(batch != NULL);

    shader_t *shader = asset_manager_get_shader(global.asset_manager, "default_shader"); 

    batch->vao           = vao_create();
    batch->vbo           = vbo_create(GL_ARRAY_BUFFER, true);
    batch->ebo           = vbo_create(GL_ELEMENT_ARRAY_BUFFER, false);
    batch->shader        = *shader; 
    batch->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));
    batch->indices       = malloc(MAX_INDICES_PER_BATCH * sizeof(u32));
    batch->texture_count = 0;
    batch->count         = 0;

    assert(batch->vertices != NULL);
    assert(batch->indices != NULL);
    memset(batch->vertices, 0, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));

    for (int i = 0; i < MAX_ENTITY_PER_BATCH; i++) {
        u32 offset = i * 4;
        u32 idx    = i * 6;
        batch->indices[idx+0] = offset + 0;
        batch->indices[idx+1] = offset + 1;
        batch->indices[idx+2] = offset + 3;
        batch->indices[idx+3] = offset + 0;
        batch->indices[idx+4] = offset + 2;
        batch->indices[idx+5] = offset + 3;
    }

    vao_bind(batch->vao);
    vbo_bind(batch->vbo);
    vbo_data(batch->vbo, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex), NULL);
    vao_attr(0, 3, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, position));
    vao_attr(1, 4, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, color));
    vao_attr(2, 2, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, tex_coord));
    vao_attr(3, 1, GL_FLOAT, sizeof(BatchVertex), offsetof(BatchVertex, tex_slot));

    vbo_bind(batch->ebo);
    vbo_data(batch->ebo, MAX_INDICES_PER_BATCH * sizeof(u32), batch->indices);
    free(batch->indices); batch->indices = NULL;

    vao_unbind();
    vbo_unbind(batch->vbo);
    vbo_unbind(batch->ebo);
    return batch;
} 

static void batch_destroy(Batch *self) {
    assert(self != NULL);

    vao_destroy(self->vao);
    vbo_destroy(self->vbo);
    vbo_destroy(self->ebo);
    shader_destroy(self->shader);
    for (u32 i = 0; i < self->texture_count; i++) texture_destroy(self->texture[i]);
    free(self);
}

static void vertices_set(Batch *batch, u32 offset, vec2s size, vec3s position, vec4s color) {
    batch->vertices[offset] = (BatchVertex) {
        .position = position,
            .color = color,
            .tex_coord = {0, 1},
            .tex_slot = -1
    };
    batch->vertices[offset+1] = (BatchVertex) {
        .position = {position.x + size.x, position.y, position.z},
            .color = color,
            .tex_coord = {1, 1},
            .tex_slot = -1
    };
    batch->vertices[offset+2] = (BatchVertex) {
        .position = {position.x, position.y + size.y, position.z},
            .color = color,
            .tex_coord = {0, 0},
            .tex_slot = -1
    };
    batch->vertices[offset+3] = (BatchVertex) {
        .position = {position.x + size.x, position.y + size.y, position.z},
            .color = color,
            .tex_coord = {1, 0},
            .tex_slot = -1
    };
}

void renderer_init(Renderer **renderer) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);

    *renderer = malloc(sizeof(**renderer));
    assert(*renderer != NULL);

    (*renderer)->total_entity   = 0;
    (*renderer)->batch_len      = 0;
    (*renderer)->batches        = malloc(BATCHES_INITIAL_CAPACITY * sizeof(Batch*));
    if ((*renderer)->batches == NULL) {
        LOG_FETAL("Failed to initialize render batch");
        abort();
    }
    
    asset_manager_push_shader(global.asset_manager, "default_shader", "../shaders/default.vert", "../shaders/default.frag");

    _world = global.world;
}

void renderer_clean(Renderer *renderer) {
    for (u32 i = 0; i < renderer->batch_len; i++) {
        memset(renderer->batches[i]->vertices, 0, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));
    }
}

void renderer_destroy(Renderer *renderer) {
    assert(renderer != NULL);
    assert(renderer->batches != NULL);

    for (u32 i = 0; i < renderer->batch_len; i++) {
        batch_destroy(renderer->batches[i]);
    }
    free(renderer->batches);
    free(renderer);

    LOG_DEBUG("Renderer destroyed");
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);
}

void renderer_append_quad(Renderer* renderer, vec2s size, vec3s position, vec4s color) {
    assert(renderer != NULL);
    assert(renderer->batches != NULL);

    s8 idx = -1;
    for (u32 i = 0; i < renderer->batch_len; i++) {
        if (renderer->batches[i]->hasRoom) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        idx = renderer->batch_len;
        renderer->batches[renderer->batch_len++] = batch_init();
    }

    vertices_set(renderer->batches[idx], renderer->batches[idx]->count * 4, size, position, color);
    renderer->batches[idx]->count++;
    renderer->total_entity++;
}

void renderer_push_texture(Renderer *renderer, texture_t texture) {
    for (size_t i = 0; i < renderer->batch_len; i++) {
        b8 has = false;
        for (size_t j = 0; j < renderer->batches[i]->texture_count; j++) {
            if (renderer->batches[i]->texture[j].handle == texture.handle) {
                has = true;
                break;
            }
        }
        if (has) {
            continue;
        }
        renderer->batches[i]->texture[renderer->batches[i]->texture_count++] = texture;
    }
}

void renderer_render(Renderer *renderer) {
    assert(renderer != NULL);
    assert(renderer->batches != NULL);

    for (u32 i = 0; i < renderer->batch_len; i++) {

        _update_ecs_state();

        vbo_bind(renderer->batches[i]->vbo);
        vbo_subdata(renderer->batches[i]->vbo, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex), renderer->batches[i]->vertices);

        for (u32 j = 0; j < renderer->batches[i]->texture_count; j++) { texture_bind(renderer->batches[i]->texture[i], j); }
        shader_bind(renderer->batches[i]->shader);
        ViewProj view_proj = get_view_proj(global.camera);
        shader_uniform_viewproj(renderer->batches[i]->shader, view_proj);
        shader_uniform_int_array(renderer->batches[i]->shader, "tex", 8, texture_slot);

        vao_bind(renderer->batches[i]->vao);
        glDrawElements(GL_TRIANGLES, (renderer->batches[i]->count * 6), GL_UNSIGNED_INT, NULL);

        vao_unbind();
        vbo_unbind(renderer->batches[i]->vbo);
        shader_unbind();
        texture_unbind();
    }
}
