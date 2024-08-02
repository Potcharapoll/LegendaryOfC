#ifndef TEXTURE_H
#define TEXTURE_H
#include "../util/types.h"

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>

typedef struct {
    GLuint handle;
    ivec2s size;
}texture_t;

texture_t texture_framebuffer(u32 width, u32 height);
texture_t texture_load(const char *path);
void texture_destroy(texture_t self);
void texture_bind(texture_t self, u32 slot);
void texture_unbind(void);
#endif
