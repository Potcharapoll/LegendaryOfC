#include <string.h>
#include "renderer.h"
#include "array_list.h"

static array_list *batches;

static Batch *batch_init(void) {
    Batch *batch = malloc(sizeof(*batch));
    batch->vao = vao_create();
    batch->vbo = vbo_create(GL_ARRAY_BUFFER, true);
    batch->ebo = vbo_create(GL_ELEMENT_ARRAY_BUFFER, false);
    batch->count = 0;
    batch->shader = shader_load("../shaders/default.vert", "../shaders/default.frag");
    batch->texture_count = 0;
    batch->vertices = malloc(MAX_VERTICES_PER_BATCH * sizeof(BatchVertex));
    batch->indices = malloc(MAX_INDICES_PER_BATCH * sizeof(u32));

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
    vao_destroy(self->vao);
    vbo_destroy(self->vbo);
    vbo_destroy(self->ebo);
    shader_destroy(self->shader);
    for (u32 i = 0; i < self->texture_count; i++) texture_destroy(self->texture[i]);
}

void renderer_init(void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);

    batches = array_list_init(sizeof(Batch), 8);
    assert(batches != NULL);
}

void renderer_destroy(void) {
    for (u32 i = 0; i < batches->len; i++) {
        Batch *batch = batches->data + i * batches->data_size;
        batch_destroy(batch);
    }
    array_list_destroy(batches);
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.2,0.2,0.2,1.0);
}

void renderer_append_quad(vec2s size, vec3s position, vec4s color) {
    Batch *batch = NULL;

    printf("%lu\n", batches->len);
    for (u32 i = 0; i < batches->len; i++) {
        batch = batches->data + i * batches->data_size;
        if (batch->count < MAX_ENTITY_PER_BATCH) break; 
    }

    if (batches->len > 0) assert(batch != NULL);

    if (batch == NULL) {
        batch = batch_init();
        array_list_append(batches, batch);
    }

    u32 offset = batch->count * 4;

    batch->count = batch->count + 1;

    batch->vertices[offset] = (BatchVertex) {
        .position = position,
        .color = color,
        .tex_coord = {0},
        .tex_slot = 0
    };
    batch->vertices[offset+1] = (BatchVertex) {
        .position = {position.x + size.x, position.y, position.z},
        .color = color,
        .tex_coord = {0},
        .tex_slot = 0
    };
    batch->vertices[offset+2] = (BatchVertex) {
        .position = {position.x, position.y + size.y, position.z},
        .color = color,
        .tex_coord = {0},
        .tex_slot = 0
    };
    batch->vertices[offset+3] = (BatchVertex) {
        .position = {position.x + size.x, position.y + size.y, position.z},
        .color = color,
        .tex_coord = {0},
        .tex_slot = 0
    };
}

void renderer_render(void) {
    for (u32 i = 0; i < batches->len; i++) {
        Batch *batch = batches->data + i * batches->data_size;

        for (u32 j = 0; j < batch->texture_count; j++) { texture_bind(batch->texture[i], i); }

        vbo_bind(batch->vbo);
        vbo_subdata(batch->vbo, MAX_VERTICES_PER_BATCH * sizeof(BatchVertex), batch->vertices);

        mat4s proj = glms_ortho(0.0f, 1280.0f, 0.0f, 768.0f, 0.0f, 100.0f);
        shader_bind(batch->shader);
        shader_uniform_mat4(batch->shader, "proj", proj);

        vao_bind(batch->vao);
        printf("%lu - %d\n", batches->len, batch->count);
        glDrawElements(GL_TRIANGLES, (batch->count * 6), GL_UNSIGNED_INT, NULL);

        vao_unbind();
        vbo_unbind(batch->vbo);
        shader_unbind();
        texture_unbind();
    }
}
