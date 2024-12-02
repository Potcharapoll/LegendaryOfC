#include "renderer.h"
#include "asset_manager.h"
#include "chunk.h"
#include "prefab.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"
#include "scene.h"

#include <string.h>

static int texture_slot[8] = {0,1,2,3,4,5,6,7};

QuadRenderer *quad_renderer_init(void) {
    QuadRenderer *renderer = malloc(sizeof(*renderer));
    ASSERT(renderer != NULL, "Failed to allocate memory for QuadRenderer", __FILE__, __LINE__);

    renderer->vertices = malloc(MAX_VERTICES_PER_BATCH * sizeof(*renderer->vertices));
    ASSERT(renderer->vertices != NULL, "Failed to allocate memory for QuadRenderer->vertices", __FILE__, __LINE__);

    renderer->shader        = *(struct Shader*)asset_manager_get_shader(global.asset_manager, "default_shader"); 
    renderer->quad_count    = 0;
    renderer->texture_count = 0;

    GL_TRY(glGenVertexArrays(1, &renderer->vao));
    GL_TRY(glGenBuffers(1, &renderer->vbo));
    GL_TRY(glGenBuffers(1, &renderer->ebo));

    GL_TRY(glBindVertexArray(renderer->vao));

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(QuadVertex), NULL, GL_DYNAMIC_DRAW));

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

    GL_TRY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo));
    GL_TRY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES_PER_BATCH * sizeof(*indices), indices, GL_STATIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, position)));
    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, color)));
    GL_TRY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, tex_coord)));
    GL_TRY(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, tex_slot)));

    GL_TRY(glEnableVertexAttribArray(0));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glEnableVertexAttribArray(2));
    GL_TRY(glEnableVertexAttribArray(3));

    GL_TRY(glBindVertexArray(0));
    free(indices);

    return renderer;
}

void quad_renderer_destroy(QuadRenderer *renderer) {
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->ebo);

    for (u8 j = 0; j < renderer->texture_count; ++j) {
        glDeleteTextures(1, &renderer->textures[j].handle);
    }

    free(renderer->vertices);
    free(renderer);
}

void quad_renderer_render(QuadRenderer *renderer) {
    if (renderer->quad_count == 0) return;

    GL_TRY(shader_bind(renderer->shader));
    GL_TRY(shader_uniform_viewproj(renderer->shader, get_view_proj(global.scene->camera)));
    GL_TRY(shader_uniform_int_array(renderer->shader, "tex", 8, texture_slot));

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo)); 
    GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(QuadVertex), renderer->vertices));

    for (u32 i = 0; i < renderer->texture_count; ++i) {
        texture_bind(renderer->textures[i], i);
    }

    GL_TRY(glBindVertexArray(renderer->vao));
    GL_TRY(glDrawElements(GL_TRIANGLES, (6 * renderer->quad_count), GL_UNSIGNED_INT, 0));

    GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
    GL_TRY(glBindVertexArray(0));
    GL_TRY(shader_unbind());

    renderer->quad_count = 0;
}

// weird?
void quad_renderer_append_prefab(QuadRenderer *renderer, vec2s coord, char *prefab_name) {
  ASSERT(global.scene->chunk != NULL, "Chunk is NULL", __FILE__, __LINE__);

  ChunkRenderInfo *render_info = global.scene->chunk->render_info;
  Prefab *prefab = prefab_get(prefab_name);

  vec3s position = { 
    render_info->position.x + TILE_SIZE * coord.x, 
    render_info->position.y + TILE_SIZE * coord.y, 
    0.0f
  };

    quad_renderer_append_quad_texture(renderer, position, prefab->size, prefab->color, prefab->spritesheet->texture, prefab->tex_coord);
}

void quad_renderer_append_quad(QuadRenderer *renderer, vec3s position, vec2s size, vec4s color) {
    f32 _tex_coord[4] = {0,1,0,1};

    u32 idx = renderer->quad_count++;
    renderer->vertices[idx * 4 + 0] = (QuadVertex){ 
        .position  = { .x = position.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[2]}, 
        .tex_slot  = -1
    };
    renderer->vertices[idx * 4 + 1] = (QuadVertex){ 
        .position  = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1], _tex_coord[2]}, 
        .tex_slot  = -1
    };
    renderer->vertices[idx * 4 + 2] = (QuadVertex){ 
        .position  = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1],_tex_coord[3]}, 
        .tex_slot  = -1
    };
    renderer->vertices[idx * 4 + 3] = (QuadVertex){ 
        .position  = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[3]}, 
        .tex_slot  = -1
    };
}

void quad_renderer_append_quad_texture(QuadRenderer *renderer, vec3s position, vec2s size, vec4s color, struct Texture texture, f32 *tex_coord) {
    f32 _tex_coord[4] = {0,1,0,1};
    u32 tex_slot      = quad_renderer_append_texture(renderer, texture);

    if (tex_coord != NULL){
        memcpy(_tex_coord, tex_coord, sizeof(f32)*4);
    }

    u32 idx = renderer->quad_count++;
    renderer->vertices[idx * 4 + 0] = (QuadVertex){ 
        .position  = { .x = position.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[2]}, 
        .tex_slot  = tex_slot 
    };
    renderer->vertices[idx * 4 + 1] = (QuadVertex){ 
        .position  = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1], _tex_coord[2]}, 
        .tex_slot  = tex_slot 
    };
    renderer->vertices[idx * 4 + 2] = (QuadVertex){ 
        .position  = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1],_tex_coord[3]}, 
        .tex_slot  = tex_slot 
    };
    renderer->vertices[idx * 4 + 3] = (QuadVertex){ 
        .position  = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[3]}, 
        .tex_slot  = tex_slot 
    };
}

u32 quad_renderer_append_texture(QuadRenderer *renderer, struct Texture texture) {
    b8 found     = false;
    u32 tex_slot = -1;

    for (u32 i = 0; i < renderer->texture_count; i++) {
        if (renderer->textures[i].handle == texture.handle) {
            tex_slot = i;
            found = true;
            break;
        }
    }

    if (!found) {
        tex_slot = renderer->texture_count++;
        renderer->textures[tex_slot] = texture;
    } 

    return tex_slot;
}

TextRenderer *text_renderer_init(ivec2s (*get_char_coord)(char)) {

    struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT);

    TextRenderer *renderer = malloc(sizeof(*renderer));
    ASSERT(renderer != NULL, "Failed to allocate memory for TextRenderer", __FILE__, __LINE__);

    renderer->vertices = malloc(MAX_VERTICES_PER_BATCH * sizeof(*renderer->vertices));
    ASSERT(renderer->vertices != NULL, "Failed to allocate memory for TextRenderer->vertices", __FILE__, __LINE__);

    renderer->shader  = *(struct Shader*)asset_manager_get_shader(global.asset_manager, "texture_shader"); 
    renderer->texture = sp->texture;
    renderer->char_count = 0;
    renderer->get_char_coord = get_char_coord;

    GL_TRY(glGenVertexArrays(1, &renderer->vao));
    GL_TRY(glGenBuffers(1, &renderer->vbo));
    GL_TRY(glGenBuffers(1, &renderer->ebo));

    GL_TRY(glBindVertexArray(renderer->vao));

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(QuadVertex), NULL, GL_DYNAMIC_DRAW));

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

    GL_TRY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo));
    GL_TRY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES_PER_BATCH * sizeof(*indices), indices, GL_STATIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, position)));
    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, color)));
    GL_TRY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, tex_coord)));
    GL_TRY(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, tex_slot)));

    GL_TRY(glEnableVertexAttribArray(0));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glEnableVertexAttribArray(2));
    GL_TRY(glEnableVertexAttribArray(3));

    GL_TRY(glBindVertexArray(0));
    FREE(indices);

    return renderer;
}

void text_renderer_destroy(TextRenderer *renderer) {
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->ebo);
    glDeleteTextures(1, &renderer->texture.handle);

    FREE(renderer->vertices);
    FREE(renderer);
}

void text_renderer_append_text(TextRenderer *renderer, char *text, vec3s position, u8 size, vec4s color) {
    f32 tex_coord[4];

    struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT);
    vec2s cell_size = {(f32)sp->cell_size.x / sp->texture.size.x, (f32)sp->cell_size.y / sp->texture.size.y};

    vec3s pos = position;
    for (u32 idx = 0; idx < strlen(text); ++idx) {

        if (text[idx] == '|') {
            pos.x = position.x;
            pos.y -= (size * 1.25);
            continue;
        }

        ivec2s char_coord = renderer->get_char_coord(text[idx]);

        tex_coord[0] = cell_size.x * char_coord.x;
        tex_coord[1] = cell_size.x * char_coord.x + cell_size.x;
        tex_coord[2] = cell_size.y * char_coord.y;
        tex_coord[3] = cell_size.y * char_coord.y + cell_size.y;

        u32 idx = renderer->char_count++;
        renderer->vertices[idx * 4 + 0] = (QuadVertex){ 
            .position  = { .x = pos.x, .y = pos.y, .z = pos.z}, 
                .color     = color, 
                .tex_coord = {tex_coord[0],tex_coord[2]}, 
        };
        renderer->vertices[idx * 4 + 1] = (QuadVertex){ 
            .position  = { .x = pos.x + size, .y = pos.y, .z = pos.z}, 
                .color     = color, 
                .tex_coord = {tex_coord[1], tex_coord[2]}, 
        };
        renderer->vertices[idx * 4 + 2] = (QuadVertex){ 
            .position  = { .x = pos.x + size, .y = pos.y + size, .z = pos.z}, 
                .color     = color, 
                .tex_coord = {tex_coord[1],tex_coord[3]}, 
        };
        renderer->vertices[idx * 4 + 3] = (QuadVertex){ 
            .position  = { .x = pos.x, .y = pos.y + size, .z = pos.z}, 
                .color     = color, 
                .tex_coord = {tex_coord[0],tex_coord[3]}, 
        };
        pos.x += (size * 0.5);
    }
}

void text_renderer_render(TextRenderer *renderer) {
    if (renderer->char_count == 0) return;

    GL_TRY(shader_bind(renderer->shader));
    GL_TRY(shader_uniform_viewproj(renderer->shader, get_view_proj(global.scene->camera)));
    GL_TRY(texture_bind(renderer->texture, 0));

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo)); 
    GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(QuadVertex), renderer->vertices));

    GL_TRY(glBindVertexArray(renderer->vao));
    GL_TRY(glDrawElements(GL_TRIANGLES, (6 * renderer->char_count), GL_UNSIGNED_INT, 0));

    GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
    GL_TRY(glBindVertexArray(0));
    GL_TRY(shader_unbind());

    renderer->char_count = 0;
}

LineRenderer *line_renderer_init(void) {
    LineRenderer *renderer = malloc(sizeof(*renderer));
    ASSERT(renderer != NULL, "Failed to allocate memory for LineRenderer", __FILE__, __LINE__);

    renderer->shader     = *(struct Shader*)asset_manager_get_shader(global.asset_manager, "line_shader"); 
    renderer->line_count = 0;
    renderer->vertices   = malloc(MAX_VERTICES_PER_BATCH * sizeof(*renderer->vertices));
    ASSERT(renderer->vertices != NULL, "Failed to allocate memory for LineRenderer->vertices", __FILE__, __LINE__);

    GL_TRY(glGenVertexArrays(1, &renderer->vao));
    GL_TRY(glGenBuffers(1, &renderer->vbo));

    GL_TRY(glBindVertexArray(renderer->vao));
    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(LineVertex), NULL, GL_DYNAMIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, position)));
    GL_TRY(glEnableVertexAttribArray(0));

    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color)));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glBindVertexArray(0));

    return renderer;
}

void line_renderer_destroy(LineRenderer *renderer) {
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteBuffers(1, &renderer->vbo);
    FREE(renderer->vertices);
    FREE(renderer);
}

void line_renderer_render(LineRenderer *renderer) {
    if (renderer->line_count == 0) return;

    GL_TRY(shader_bind(renderer->shader));
    GL_TRY(shader_uniform_viewproj(renderer->shader, get_view_proj(global.scene->camera)));

    GL_TRY(glBindVertexArray(renderer->vao));
    GL_TRY(glDrawArrays(GL_LINES, 0, (renderer->line_count * 2)));
    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo));
    GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(struct LineVertex), renderer->vertices));
    GL_TRY(glBindVertexArray(0));
    GL_TRY(shader_unbind());

    renderer->line_count = 0;
}

void line_renderer_append_line_segment(LineRenderer *renderer, vec2s start, vec2s end, vec4s color) {
    u32 idx = renderer->line_count++;

    renderer->vertices[idx * 2 + 0] = (LineVertex){ 
        .position  = start,
        .color     = color, 
    };
    renderer->vertices[idx * 2 + 1] = (LineVertex){ 
        .position  = end,
        .color     = color, 
    };
}

void line_renderer_append_quad_line(LineRenderer *renderer, vec2s position, vec2s size, vec4s color) {
    vec2s points[] = {
        {position.x - size.x, position.y - size.y},   
        {position.x + size.x, position.y - size.y},   
        {position.x + size.x, position.y + size.y},   
        {position.x - size.x, position.y + size.y},   
    };

    line_renderer_append_line_segment(renderer, points[0], points[1], color);
    line_renderer_append_line_segment(renderer, points[1], points[2], color);
    line_renderer_append_line_segment(renderer, points[2], points[3], color);
    line_renderer_append_line_segment(renderer, points[3], points[0], color);
}

void line_renderer_append_aabb(LineRenderer *renderer, AABB aabb, vec4s color) {
    line_renderer_append_quad_line(renderer, aabb.center, aabb.half_size, color);
}
