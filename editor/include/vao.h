#ifndef VAO_H
#define VAO_H
#include <stdio.h>
#include <glad/glad.h>

struct VAO {
    GLuint handle;
};

struct VAO vao_create(void);
void vao_destroy(struct VAO self);
void vao_bind(struct VAO self);
void vao_unbind(void);
void vao_attr(GLint index, GLuint size, GLenum type, size_t stride, size_t offset);
#endif
