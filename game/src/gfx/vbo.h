#ifndef VBO_H
#define VBO_H
#include <glad/glad.h>
#include "../util/types.h"

typedef struct {
    GLuint handle;
    GLenum type;
    b8 dynamic;
}VBO;

VBO vbo_create(GLenum type, b8 dynamic);
void vbo_destroy(VBO self);
void vbo_bind(VBO self);
void vbo_unbind(VBO self);
void vbo_data(VBO self, size_t size, void *data);
void vbo_subdata(VBO self, size_t size, void *data);
#endif
