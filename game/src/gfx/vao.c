#include "vao.h"

VAO vao_create(void) {
    VAO vao;
    glGenVertexArrays(1, &vao);
    return vao;
}

void vao_destroy(VAO self) {
    glDeleteBuffers(1, &self);
}

void vao_bind(VAO self) {
    glBindVertexArray(self);
}

void vao_unbind(void) {
    glBindVertexArray(0);
}

void vao_attr(GLint index, GLuint size, GLenum type, size_t stride, size_t offset) {
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset);
    glEnableVertexAttribArray(index);
}

