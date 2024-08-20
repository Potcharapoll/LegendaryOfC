#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cglm/struct.h>
#include <glad/glad.h>
#include "types.h"

#define FONT1_PATH  "fonts/JetBrainsMonoNerdFont-Medium.ttf"
#define FONT2_PATH  "fonts/UniversCondensed.ttf"
#define FONT3_PATH  "fonts/Raleway-Bold.ttf"
#define FONT4_PATH  "fonts/Raleway-Medium.ttf"
#define FONT5_PATH  "fonts/PressStart2P-Regular.ttf"
#define FONT_HEIGHT 14
#define ASCII_COUNT 127

#define MAX_QUAD_PER_BATCH 10000
#define MAX_VERTICES_PER_BATCH 40000
#define MAX_INDICES_PER_BATCH 60000

#define DIALOG_TEXT_BOX_POSITION (vec3s){100.0f,  25.0f, 0.0f}
#define DIALOG_NAME_BOX_POSITION (vec3s){100.0f, 255.0f, 0.0f}
#define DIALOG_TRIANGLE_POSITION (vec3s){WIDTH - 165.0f, 70.0f, 0.0f}
#define DIALOG_TEXT_POSITION     (vec3s){150.0f, 145.0f, 0.0f}
#define DIALOG_NAME_POSITION     (vec3s){180.0f, 195.0f, 0.0f}
#define DIALOG_QUESTION_POSITION (vec3s){150.0f, 145.0f, 0.0f}
#define DIALOG_ANSWER1_POSITION  (vec3s){240.0f, 100.0f, 0.0f}
#define DIALOG_ANSWER2_POSITION  (vec3s){240.0f,  60.0f, 0.0f}
#define DIALOG_ANSWER3_POSITION  (vec3s){640.0f, 100.0f, 0.0f}
#define DIALOG_ANSWER4_POSITION  (vec3s){640.0f,  60.0f, 0.0f}
#define DIALOG_TEXT_SIZE         (vec2s){WIDTH - 200.0f, 200.0f}
#define DIALOG_NAME_SIZE         (vec2s){200, 50}
#define DIALOG_TRIANGLE_SIZE     (vec2s){15, 15}
#define DIALOG_COLOR             (vec4s){0.5, 0.6, 1.0, 0.8}
#define DIALOG_TEXT_COLOR        (vec4s){1.0, 1.0, 1.0, 0.8}

// Choice 1: Raleway-Bold with 28px
// Choice 2: PressStart2P-Regular with

struct Vertex {
    vec3s position;
    vec4s color;
    vec2s tex_coord;
    f32   tex_slot; // -1 for none, and 0 - 8 slot of texture
};

struct Character {
    GLuint texture;
    ivec2s bearing;
    ivec2s size;
    u32    advance;
    vec4s  uvs; // left (2), right (2)
};

struct BatchRender {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shader;

    GLuint textures[8];
    u32 texture_count;

    u32 quad_count;
    struct Vertex *vertices;
};

struct RenderState {
    struct BatchRender *_render_batch;
    struct BatchRender *_dialog_batch;

    struct {
        FT_Library handle;
        FT_Face    face;

        GLuint texture_atlas;

        struct Character *characters; 
        u32 character_count;
    } font;

    mat4s proj;
};

GLuint _renderer_compile_shader(GLenum type, char *path);
void _renderer_init_dialog(void);
#endif
