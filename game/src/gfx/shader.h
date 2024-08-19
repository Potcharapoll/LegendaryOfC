#ifndef SHADER_H
#define SHADER_H
#include <glad/glad.h>
#include <cglm/struct.h>
#include "../core/camera.h"

struct Shader{
    GLuint handle, vs, fs;
};

struct Shader shader_load(char *vs_path, char *fs_path);
void shader_bind(struct Shader self);
void shader_unbind(void);
void shader_destroy(struct Shader self);
void shader_uniform_mat4(struct Shader self, char *name, mat4s m);
void shader_uniform_float(struct Shader self, char *name, float f);
void shader_uniform_int(struct Shader self, char *name, int i);
void shader_uniform_int_array(struct Shader self, char *name, int count, int arr[]);
void shader_uniform_vec2(struct Shader self, char *name, vec2s v);
void shader_uniform_vec4(struct Shader self, char *name, vec4s v);
void shader_uniform_viewproj(struct Shader self, struct ViewProj view_proj);
#endif
