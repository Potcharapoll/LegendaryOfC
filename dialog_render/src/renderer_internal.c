#include "renderer_internal.h"
#include "global.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void _renderer_init_dialog(void) {
    glGenTextures(1, &global.DialogState.dialog_texture);
    glBindTexture(GL_TEXTURE_2D, global.DialogState.dialog_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(1);

    s32 width, height, bpp;
    u8 *pixels = stbi_load("images/dialog_box.png", &width, &height, &bpp, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);
}

GLuint _renderer_compile_shader(GLenum type, char *path) {
    FILE *fp;
    long len;
    char *txt;

    fp = fopen(path, "rb");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    assert(len > 0);

    txt = malloc(len);
    assert(txt != NULL);

    fseek(fp, 0, SEEK_SET);
    fread(txt, 1, len, fp);
    assert(strlen(txt) > 0);

    fclose(fp);

    GLuint shader = glCreateShader(type);
    GL_TRY(glShaderSource(shader, 1, (const GLchar * const *)&txt, (const GLint *)&len));
    GL_TRY(glCompileShader(shader));

    GLint rel;
    GL_TRY(glGetShaderiv(shader, GL_COMPILE_STATUS, &rel));
    if (!rel) {
        char msg[512];
        GL_TRY(glGetShaderInfoLog(shader, 512, NULL, msg));
        fprintf(stderr, "%s:Error:%s\n", (type == GL_VERTEX_SHADER) ? "VertexShader" : "FragmentShader", msg);
    }

    free(txt);
    return shader;
}
