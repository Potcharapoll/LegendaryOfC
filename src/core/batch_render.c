#include "batch_render.h"
#include "../util/debug.h"
#include "../global.h"

#include <string.h>

static int texture_slot[8] = {0,1,2,3,4,5,6,7};

struct BatchRender *batch_render_init(void) {
    struct BatchRender *new_batch = malloc(sizeof(*new_batch));

    new_batch->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(*new_batch->vertices));
    new_batch->quad_count    = 0;
    new_batch->texture_count = 0;
    new_batch->shader        = *(struct Shader*)asset_manager_get_shader(global.asset_manager, "default_shader"); 

    memset(new_batch->vertices, 0, MAX_VERTICES_PER_BATCH * sizeof(*new_batch->vertices));
    memset(new_batch->textures, 0, sizeof(new_batch->textures));

    GL_TRY(glGenVertexArrays(1, &new_batch->vao));
    GL_TRY(glGenBuffers(1, &new_batch->vbo));
    GL_TRY(glGenBuffers(1, &new_batch->ebo));

    GL_TRY(glBindVertexArray(new_batch->vao));

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, new_batch->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), NULL, GL_DYNAMIC_DRAW));

    u32 *indices = malloc(MAX_INDICES_PER_BATCH * sizeof(*indices));
    for (int i = 0; i < MAX_QUAD_PER_BATCH; i++) {
        u32 offset = i * 4;
        u32 idx    = i * 6;

        indices[idx + 0] = offset + 0;
        indices[idx + 1] = offset + 1;
        indices[idx + 2] = offset + 2;

        indices[idx + 3] = offset + 2;
        indices[idx + 4] = offset + 3;
        indices[idx + 5] = offset + 0;
    }

    GL_TRY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_batch->ebo));
    GL_TRY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES_PER_BATCH * sizeof(*indices), indices, GL_STATIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, position)));
    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, color)));
    GL_TRY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, tex_coord)));
    GL_TRY(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, tex_slot)));

    GL_TRY(glEnableVertexAttribArray(0));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glEnableVertexAttribArray(2));
    GL_TRY(glEnableVertexAttribArray(3));

    GL_TRY(glBindVertexArray(0));

    free(indices);
    return new_batch;
}

void batch_render_destroy(struct BatchRender *batch) {
    glDeleteVertexArrays(1, &batch->vao);
    glDeleteBuffers(1, &batch->vbo);
    glDeleteBuffers(1, &batch->ebo);

    free(batch->vertices);
    free(batch);
}

void batch_render_render(struct BatchRender *batch) {
    shader_bind(batch->shader);
    shader_uniform_viewproj(batch->shader, get_view_proj(global.camera));
    shader_uniform_int_array(batch->shader, "tex", 8, texture_slot);

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, batch->vbo));
    GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), batch->vertices));

    for (u32 i = 0; i < batch->texture_count; i++) {
        texture_bind(batch->textures[i], i);
    }

    GL_TRY(glBindVertexArray(batch->vao));
    GL_TRY(glDrawElements(GL_TRIANGLES, (6 * batch->quad_count), GL_UNSIGNED_INT, 0));

    GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
    GL_TRY(glBindVertexArray(0));

    shader_unbind();
}

// add texture to batch render and return the texture slot
u32 batch_render_append_texture(struct BatchRender *batch, struct Texture texture) {
    b8 found     = false;
    u32 tex_slot = -1;

    for (u32 i = 0; i < batch->texture_count; i++) {
        if (batch->textures[i].handle == texture.handle) {
            tex_slot = i;
            found = true;
            break;
        }
    }

    if (!found) {
        tex_slot = batch->texture_count++;
        batch->textures[tex_slot] = texture;
    } 

    return tex_slot;
}

// return texture slot of texture in batch render (Asuume that it was already added to the batch render overwise will return -1)
u32 batch_render_get_texture_slot(struct BatchRender *batch, struct Texture texture) {
    for (u32 i = 0; i < batch->texture_count; i++) {
        if (batch->textures[i].handle == texture.handle) {
            return i;
        }
    }
    return -1;
}

// append quad to _render_batch
void batch_render_append_quad(struct BatchRender *batch, vec3s position, vec2s size, vec4s color) {
    u32 idx = batch->quad_count++;

    batch->vertices[idx * 4 + 0] = (struct Vertex){ 
        .position = { .x = position.x, .y = position.y, .z = position.z}, 
        .color    = color, 
        .tex_slot = -1 
    };
    batch->vertices[idx * 4 + 1] = (struct Vertex){ 
        .position = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color    = color, 
        .tex_slot = -1 
    };
    batch->vertices[idx * 4 + 2] = (struct Vertex){ 
        .position = { .x = position.x + size.x, .y = position.y + size.y , .z = position .z}, 
        .color    = color, 
        .tex_slot = -1 
    };
    batch->vertices[idx * 4 + 3] = (struct Vertex){ 
        .position = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color    = color, 
        .tex_slot = -1 
    };
}

// append quad with texture to _render_batch
void batch_render_append_quad_texture(struct BatchRender *batch, vec3s position, vec2s size, vec4s color, struct Texture texture, f32 *tex_coord) {
    f32 _tex_coord[4] = {0,1,0,1};

    if (batch->texture_count >= 8) {
        fprintf(stderr, "Texture slot fulled\n");
        return;
    }

    u32 tex_slot = batch_render_get_texture_slot(batch, texture);
    if (tex_slot == (u32)-1) {
        tex_slot = batch_render_append_texture(batch, texture);
    }

    if (tex_coord != NULL){
        memcpy(_tex_coord, tex_coord, sizeof(f32)*4);
    }

    u32 idx = batch->quad_count++;

    batch->vertices[idx * 4 + 0] = (struct Vertex){ 
        .position  = { .x = position.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[2]}, 
        .tex_slot  = tex_slot 
    };
    batch->vertices[idx * 4 + 1] = (struct Vertex){ 
        .position  = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1], _tex_coord[2]}, 
        .tex_slot  = tex_slot 
    };
    batch->vertices[idx * 4 + 2] = (struct Vertex){ 
        .position  = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1],_tex_coord[3]}, 
        .tex_slot  = tex_slot 
    };
    batch->vertices[idx * 4 + 3] = (struct Vertex){ 
        .position  = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[3]}, 
        .tex_slot  = tex_slot 
    };
}
