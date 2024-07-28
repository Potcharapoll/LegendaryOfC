#include "vao.h"

struct VAO vao_create(void) {
    struct VAO vao;
    glGenVertexArrays(1, &vao.handle);
    return vao;
}

void vao_destroy(struct VAO self) {
    glDeleteBuffers(1, &self.handle);
}

void vao_bind(struct VAO self) {
    glBindVertexArray(self.handle);
}

void vao_unbind(void) {
    glBindVertexArray(0);
}

void vao_attr(GLint index, GLuint size, GLenum type, size_t stride, size_t offset) {
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset);
    glEnableVertexAttribArray(index);
}

