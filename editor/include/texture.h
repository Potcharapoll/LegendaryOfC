#ifndef TEXTURE_H
#define TEXTURE_H
#include "types.h"

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>

struct Texture {
    GLuint handle;
    ivec2s size;
};

struct Texture texture_framebuffer(u32 width, u32 height);
struct Texture texture_load(const char *path);
void texture_destroy(struct Texture self);
void texture_bind(struct Texture self, u32 slot);
void texture_unbind(void);
#endif
