#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "texture.h"
#include "types.h"

struct Framebuffer {
    u32 handle;
    u32 width, height;

    struct {
        u32 handle;
    } rbo;

    struct Texture texture;
};

struct Framebuffer framebuffer_create(u32 width, u32 height); 
void framebuffer_delete(struct Framebuffer self);
void framebuffer_bind(struct Framebuffer self);
void framebuffer_unbind();
#endif
