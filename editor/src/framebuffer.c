#include "framebuffer.h"
#include "texture.h"
#include <glad/glad.h>

struct Framebuffer framebuffer_create(u32 width, u32 height) {
    struct Framebuffer fb = {
        .width = width,
        .height = height,
        .texture = texture_framebuffer(width, height)
    };

    glGenFramebuffers(1, &fb.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.handle);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture.handle, 0);

    // render buffer object
    glGenRenderbuffers(1, &fb.rbo.handle);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo.handle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

    // attach render buffer object to framebuffer object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.rbo.handle);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer is not complete");
        abort();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return fb;
}

void framebuffer_delete(struct Framebuffer self) {
    texture_destroy(self.texture); 
    glDeleteRenderbuffers(1, &self.rbo.handle);
    glDeleteFramebuffers(1, &self.handle);
}

void framebuffer_bind(struct Framebuffer self) {
    glBindFramebuffer(GL_FRAMEBUFFER, self.handle);
}
void framebuffer_unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
