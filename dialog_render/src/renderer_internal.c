#include "renderer_internal.h"
#include "global.h"

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
