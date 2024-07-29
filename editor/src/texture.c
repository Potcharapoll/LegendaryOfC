#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Texture texture_framebuffer(u32 width, u32 height) {
    struct Texture texture = { .size = {width, height} };

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

struct Texture texture_load(const char *path) {
    struct Texture texture;

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    int width, height, bpp;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *pixels = stbi_load(path, &width, &height, &bpp, 0);
    if (pixels == NULL) {
        fprintf(stderr, "Failed to image from path \'%s\'\n", path);
        exit(1);
    }

    texture.size = (ivec2s){width, height};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void texture_destroy(struct Texture self) {
    glDeleteTextures(1, &self.handle);
}

void texture_bind(struct Texture self, u32 slot) {
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_2D, self.handle);
}
void texture_unbind(void) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

