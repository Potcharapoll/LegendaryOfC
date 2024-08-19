#include "shader.h"
#include "../global.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <glad/glad.h>

static GLuint _compile(GLenum type, char *path) {
    FILE *fp;
    long len;
    char *txt;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("cannot open file at %s\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    assert(len > 0);
    fseek(fp, 0, SEEK_SET);
    txt = malloc(len);
    assert(txt != NULL);
    fread(txt, 1, len, fp);
    fclose(fp);

    GLuint shader = glCreateShader(type);
    GL_TRY(glShaderSource(shader, 1, (const GLchar* const *)&txt, (const GLint*)&len));
    GL_TRY(glCompileShader(shader));
    
    int check;
    GL_TRY(glGetShaderiv(shader, GL_COMPILE_STATUS, &check));
    if(check == GL_FALSE) {
        char log[512];
        GL_TRY(glGetShaderInfoLog(shader, 512, NULL, log));
        puts(log);
    }
    free(txt);
    return shader;
}

struct Shader shader_load(char *vs_path, char *fs_path) {
    struct Shader shader = {
        .handle = glCreateProgram(),
        .vs = _compile(GL_VERTEX_SHADER, vs_path),
        .fs = _compile(GL_FRAGMENT_SHADER, fs_path),
    };
    GL_TRY(glAttachShader(shader.handle, shader.vs));
    GL_TRY(glAttachShader(shader.handle, shader.fs));

    int check;
    GL_TRY(glLinkProgram(shader.handle));
    GL_TRY(glGetProgramiv(shader.handle, GL_LINK_STATUS, &check));
    if(check == GL_FALSE) {
        char log[512];
        GL_TRY(glGetProgramInfoLog(shader.handle, 512, NULL, log));
        puts(log);
    }

    GL_TRY(glValidateProgram(shader.handle));
    GL_TRY(glGetProgramiv(shader.handle, GL_VALIDATE_STATUS, &check));
    if(check == GL_FALSE) {
        char log[512];
        GL_TRY(glGetProgramInfoLog(shader.handle, 512, NULL, log));
        puts(log);
    }
    return shader;
}

void shader_bind(struct Shader self) {
    GL_TRY(glUseProgram(self.handle));
}

void shader_unbind(void) {
    GL_TRY(glUseProgram(0));
}

void shader_destroy(struct Shader self) {
    GL_TRY(glDeleteProgram(self.handle));
    GL_TRY(glDeleteShader(self.vs));
    GL_TRY(glDeleteShader(self.fs);) 
}

void shader_uniform_mat4(struct Shader self, char *name, mat4s m) {
    GL_TRY(glUniformMatrix4fv(glGetUniformLocation(self.handle, name), 1, GL_FALSE, (const GLfloat*)m.raw));
}

void shader_uniform_float(struct Shader self, char *name, float f) {
    GL_TRY(glUniform1f(glGetUniformLocation(self.handle, name), f));
}

void shader_uniform_int(struct Shader self, char *name, int i) {
    GL_TRY(glUniform1i(glGetUniformLocation(self.handle, name), i));
}

void shader_uniform_vec2(struct Shader self, char *name, vec2s v) {
    GL_TRY(glUniform2f(glGetUniformLocation(self.handle, name), v.raw[0], v.raw[1]));
}

void shader_uniform_vec4(struct Shader self, char *name, vec4s v) {
    GL_TRY(glUniform4f(glGetUniformLocation(self.handle, name), v.raw[0], v.raw[1], v.raw[2], v.raw[3]));
}

void shader_uniform_viewproj(struct Shader self, struct ViewProj view_proj) {
    GL_TRY(glUniformMatrix4fv(glGetUniformLocation(self.handle, "proj"), 1, GL_FALSE, (const GLfloat*)view_proj.proj.raw));
    GL_TRY(glUniformMatrix4fv(glGetUniformLocation(self.handle, "view"), 1, GL_FALSE, (const GLfloat*)view_proj.view.raw));
}

void shader_uniform_int_array(struct Shader self, char *name, int count, int arr[]) {
    GL_TRY(glUniform1iv(glGetUniformLocation(self.handle, name), count, arr));
}
