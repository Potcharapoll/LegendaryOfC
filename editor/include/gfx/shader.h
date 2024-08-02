#ifndef SHADER_H
#define SHADER_H
#include <glad/glad.h>
#include <cglm/struct.h>
#include "../core/camera.h"

typedef struct {
    GLuint handle, vs, fs;
} shader_t;

shader_t shader_load(char *vs_path, char *fs_path);
void shader_bind(shader_t self);
void shader_unbind(void);
void shader_destroy(shader_t self);
void shader_uniform_mat4(shader_t self, char *name, mat4s m);
void shader_uniform_float(shader_t self, char *name, float f);
void shader_uniform_int(shader_t self, char *name, int i);
void shader_uniform_int_array(shader_t self, char *name, int count, int arr[]);
void shader_uniform_vec2(shader_t self, char *name, vec2s v);
void shader_uniform_viewproj(shader_t self, ViewProj view_proj);
#endif
