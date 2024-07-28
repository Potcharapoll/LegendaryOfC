#include "shader.h"

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
    glShaderSource(shader, 1, (const GLchar* const *)&txt, (const GLint*)&len);
    glCompileShader(shader);
    
    int check;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &check);
    if(check == GL_FALSE) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
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
    glAttachShader(shader.handle, shader.vs);
    glAttachShader(shader.handle, shader.fs);

    int check;
    glLinkProgram(shader.handle);
    glGetProgramiv(shader.handle, GL_LINK_STATUS, &check);
    if(check == GL_FALSE) {
        char log[512];
        glGetProgramInfoLog(shader.handle, 512, NULL, log);
        puts(log);
    }

    glValidateProgram(shader.handle);
    glGetProgramiv(shader.handle, GL_VALIDATE_STATUS, &check);
    if(check == GL_FALSE) {
        char log[512];
        glGetProgramInfoLog(shader.handle, 512, NULL, log);
        puts(log);
    }
    return shader;
}

void shader_bind(struct Shader self) {
    glUseProgram(self.handle);
}

void shader_unbind(void) {
    glUseProgram(0);
}

void shader_destroy(struct Shader self) {
    glDeleteProgram(self.handle);
    glDeleteShader(self.vs);
    glDeleteShader(self.fs); 
}

void shader_uniform_mat4(struct Shader self, char *name, mat4s m) {
    glUniformMatrix4fv(glGetUniformLocation(self.handle, name), 1, GL_FALSE, (const GLfloat*)m.raw);
}

void shader_uniform_float(struct Shader self, char *name, float f) {
    glUniform1f(glGetUniformLocation(self.handle, name), f);
}

void shader_uniform_int(struct Shader self, char *name, int i) {
    glUniform1i(glGetUniformLocation(self.handle, name), i);
}

void shader_uniform_vec2(struct Shader self, char *name, vec2s v) {
    glUniform2f(glGetUniformLocation(self.handle, name), v.raw[0], v.raw[1]);
}

/* void shader_uniform_viewproj(struct Shader self, struct ViewProj view_proj) { */
/*     glUniformMatrix4fv(glGetUniformLocation(self.handle, "proj"), 1, GL_FALSE, (const GLfloat*)view_proj.proj.raw); */
/*     glUniformMatrix4fv(glGetUniformLocation(self.handle, "view"), 1, GL_FALSE, (const GLfloat*)view_proj.view.raw); */
/* } */

void shader_uniform_int_array(struct Shader self, char *name, int count, int arr[]) {
    glUniform1iv(glGetUniformLocation(self.handle, name), count, arr);
}
