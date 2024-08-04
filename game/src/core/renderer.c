#include <string.h>
#include "renderer.h"
#include "asset_manager.h"
#include "camera.h"
#include "../util/log.h"

static int texture_slot[8] = {0,1,2,3,4,5,6,7};

static AssetManager *_asset_manager = NULL;
static Camera *_camera = NULL;
static Batch *_batches = NULL;

static void batch_init(Batch *batch) {
    batch->vao           = vao_create();
    batch->vbo           = vbo_create(GL_ARRAY_BUFFER, true);
    batch->ebo           = vbo_create(GL_ELEMENT_ARRAY_BUFFER, false);
    batch->shader        = *(Shader*)asset_manager_get_shader(_asset_manager, "default_shader"); 
    batch->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));
    batch->indices       = malloc(MAX_INDICES_PER_BATCH * sizeof(u32));
    batch->texture_count = 0;
    batch->count         = 0;
    batch->hasRoom       = true;

    ASSERT_MSG(batch->vertices == NULL, "Failed to allocate memory for vertices");
    ASSERT_MSG(batch->indices == NULL, "Failed to allocate memory for indices");

    for (int i = 0; i < MAX_QUAD_PER_BATCH; i++) {
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

void renderer_init(Camera *camera, AssetManager *assetmanager) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);

    _batches = malloc(LAYER_COUNT * sizeof(*_batches));
    ASSERT_MSG(_batches == NULL, "Failed to allocate memory for batches");

    _asset_manager = assetmanager;
    _camera        = camera;
    asset_manager_push_shader(_asset_manager, "default_shader", "../res/shaders/default.vert", "../res/shaders/default.frag");

    for (int i = 0; i < LAYER_COUNT; i++) batch_init(&_batches[i]);
}

void renderer_clean(void) {
    for (u32 i = 0; i < LAYER_COUNT; i++) {
        memset(&_batches[i].vertices, 0, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));
    }
}

void renderer_destroy(void) {
    for (int i = 0; i < LAYER_COUNT; i++) { 
        vao_destroy(_batches[i].vao);
        vbo_destroy(_batches[i].vbo);
        vbo_destroy(_batches[i].ebo);
        shader_destroy(_batches[i].shader);

        for (u32 i = 0; i < _batches[i].texture_count; i++) texture_destroy(_batches[i].texture[i]);
    }

    free(_batches);
    LOG_DEBUG("Renderer destroyed");
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);
}

void renderer_append_quad(RenderLayer layer, vec2s size, vec3s position, vec4s color) {
    if (!_batches[layer].hasRoom) {
        LOG_ERROR("Layer %d is full", layer);
        return;
    }

    vertices_set(&_batches[layer], _batches[layer].count * 4, size, position, color);
    _batches[layer].count++;
}

// push texture to all layer of render batch
void renderer_push_texture(texture_t texture) {
    for (size_t i = 0; i < LAYER_COUNT; i++) {
        b8 has = false;
        for (size_t j = 0; j < _batches[i].texture_count; j++) {
            if (_batches[i].texture[j].handle == texture.handle) {
                has = true;
                break;
            }
        }
        if (has) {
            continue;
        }
        _batches[i].texture[_batches[i].texture_count++] = texture;
    }
}

void renderer_render(void) {
    for (u32 i = 0; i < LAYER_COUNT; i++) {
        vbo_bind(_batches[i].vbo);
        vbo_subdata(_batches[i].vbo, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex), _batches[i].vertices);

        for (u32 j = 0; j < _batches[i].texture_count; j++) { texture_bind(_batches[i].texture[i], j); }
        shader_bind(_batches[i].shader);
        ViewProj view_proj = get_view_proj(_camera);
        shader_uniform_viewproj(_batches[i].shader, view_proj);
        shader_uniform_int_array(_batches[i].shader, "tex", 8, texture_slot);

        vao_bind(_batches[i].vao);
        glDrawElements(GL_TRIANGLES, (_batches[i].count * 6), GL_UNSIGNED_INT, NULL);

        vao_unbind();
        vbo_unbind(_batches[i].vbo);
        shader_unbind();
        texture_unbind();
    }
}
