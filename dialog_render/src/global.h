#ifndef GLOBAL_H
#define GLOBAL_H
#include "types.h"

#include <stdio.h>
#include <glad/glad.h>
#include <cglm/struct.h>

#define WIDTH  1280
#define HEIGHT  768
#define TITLE  "Dialog_Render"

struct Global {
    struct Window *window; 
    f32 dt;

    struct {
        char *name;
        struct DialogNode *curr_dialog_node;

        u32 curr_animation_idx;
        u32 selected_idx;

        b8  onAnimation;
    } DialogState;

    struct {
        GLuint atlas;
        s32    atlas_width, atlas_height;
        s32    glyph_width, glyph_height;
        u32    rows, cols;
    } fonts;

    f32 input_delay;

    mat4s proj;
};

extern struct Global global;

static inline void gl_check_err(char *file, s32 line) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        char *err_string = NULL;

        switch (err) {
            case GL_INVALID_ENUM:                  err_string = "INVALID ENUM"; break;
            case GL_INVALID_VALUE:                 err_string = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             err_string = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 err_string = "OUT OF MEMORY"; break; 
            case GL_INVALID_FRAMEBUFFER_OPERATION: err_string = "INVALID FRAMEBUFFER OPERATION"; break;
        }

        fprintf(stderr, "OpenGL Error:%s: %s (%d)\n", err_string, file, line);
    }
}

#define GL_TRY(x) x; gl_check_err(__FILE__, __LINE__);
#endif
