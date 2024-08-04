#ifndef SHADER_H
#define SHADER_H
#include <glad/glad.h>
#include <cglm/struct.h>
#include "../core/camera.h"

typedef struct {
    GLuint handle, vs, fs;
} Shader;

Shader shader_load(char *vs_path, char *fs_path);
void shader_bind(Shader self);
void shader_unbind(void);
void shader_destroy(Shader self);
void shader_uniform_mat4(Shader self, char *name, mat4s m);
void shader_uniform_float(Shader self, char *name, float f);
void shader_uniform_int(Shader self, char *name, int i);
void shader_uniform_int_array(Shader self, char *name, int count, int arr[]);
void shader_uniform_vec2(Shader self, char *name, vec2s v);
void shader_uniform_viewproj(Shader self, ViewProj view_proj);
#endif
