#ifndef GLOBAL_H
#define GLOBAL_H
#include "core/asset_manager.h"
#include "gfx/window.h"
#include "core/chunk.h"
#include "util/types.h"

#define CHUNK1_SPAWN_X 600 
#define CHUNK1_SPAWN_Y 270

struct Global {
    struct Window        *window;
    struct Camera        *camera;
    struct AssetManager  *asset_manager;

    f32 dt;

    struct {
        Chunk *chunk;
        u32   chunk_idx;
        b8    next_chunk;
    } ChunkState;

    struct {
        b8 render_dialog;
        b8 render_quad_line;
    } RenderState;

    struct {
    } PlayerState;
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
