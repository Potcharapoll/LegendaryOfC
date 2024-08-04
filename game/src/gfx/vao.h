#ifndef VAO_H
#define VAO_H
#include <stdio.h>
#include <glad/glad.h>

typedef GLuint VAO;

VAO vao_create(void);
void vao_destroy(VAO self);
void vao_bind(VAO self);
void vao_unbind(void);
void vao_attr(GLint index, GLuint size, GLenum type, size_t stride, size_t offset);
#endif
