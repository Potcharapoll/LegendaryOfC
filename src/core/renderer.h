#ifndef RENDERER_H
#define RENDERER_H
#define MAX_QUAD_PER_BATCH      5000
#define MAX_VERTICES_PER_BATCH 20000
#define MAX_INDICES_PER_BATCH  30000
#include "../gfx/texture.h"
#include "../gfx/shader.h"
#include "physics.h"

#include <cglm/struct.h>

typedef struct QuadVertex {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32   tex_slot; // -1 for none, and 0 - 8 slot of texture
}QuadVertex;

typedef struct LineVertex {
    vec2s position;
    vec4s color;
}LineVertex;

typedef struct QuadRenderer {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    struct Shader shader;

    u32 texture_count;
    struct Texture textures[8];

    u32 quad_count;
    QuadVertex *vertices;
}QuadRenderer;

typedef struct TextRenderer {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    struct Shader shader;
    struct Texture texture;

    u32 char_count;
    QuadVertex *vertices;

    // function to get char coordinate
    ivec2s (*get_char_coord)(char);
}TextRenderer;

typedef struct LineRenderer {
    GLuint vao;
    GLuint vbo;
    struct Shader shader;

    u32 line_count;
    LineVertex *vertices;
}LineRenderer;

// Honestly I was concerned about naming between renderer or batch_render 
// however I decided to use renderer instead of batch_render

QuadRenderer *quad_renderer_init(void);
void quad_renderer_destroy(QuadRenderer *renderer);
void quad_renderer_append_prefab(QuadRenderer *renderer, vec2s coord, char *prefab_name);
void quad_renderer_append_quad(QuadRenderer *renderer, vec3s position, vec2s size, vec4s color);
void quad_renderer_append_quad_texture(QuadRenderer *renderer, vec3s position, vec2s size, vec4s color, struct Texture texture, f32 *tex_coord);
u32 quad_renderer_append_texture(QuadRenderer *renderer, struct Texture texture);
void quad_renderer_render(QuadRenderer *renderer);

TextRenderer *text_renderer_init(ivec2s (*get_char_coord)(char));
void text_renderer_destroy(TextRenderer *renderer);
void text_renderer_append_text(TextRenderer *renderer, char *text, vec3s position, u8 size, vec4s color);
void text_renderer_render(TextRenderer *renderer);

LineRenderer *line_renderer_init(void);
void line_renderer_destroy(LineRenderer *renderer);
void line_renderer_append_line_segment(LineRenderer *renderer, vec2s start, vec2s end, vec4s color);
void line_renderer_append_quad_line(LineRenderer *renderer, vec2s position, vec2s size, vec4s color);
void line_renderer_append_aabb(LineRenderer *renderer, AABB aabb, vec4s color); 
void line_renderer_render(LineRenderer *renderer);
#endif
