#ifndef DEBUG_H
#define DEBUG_H

#define INFO_CHUNK(chunk) \
    printf("Chunk {\n" \
           "\tstart_position: (%f,%f),\n" \
           "\tend_position  : (%f,%f),\n" \
           "\ttilemap       : %p,\n"      \
           "\tgrid_size     : (%u,%u)\n}\n", \
            chunk->start_position.x, chunk->start_position.y, \
            chunk->end_position.x,   chunk->end_position.y, \
            chunk->tilemap, chunk->grid_size.x, chunk->grid_size.y);

#define INFO_BATCH(batch) \
    printf("Batch {\n" \
           "\tvao       : %u,\n"   \
           "\tvbo       : %u,\n"   \
           "\tebo       : %u,\n"   \
           "\ttex_count : %u\n}\n",\
           batch->vao, batch->vbo, batch->ebo, batch->texture_count);

#define INFO_CAMERA(camera) \
    printf("Camera {\n"  \
           "\tposition: (%f,%f,%f)\n" \
           "}\n", \
           camera->position.x, camera->position.y, camera->position.z);

#define gl_check_err(file, line) \
    { \
        GLenum err;\
        while ((err = glGetError()) != GL_NO_ERROR) { \
            char *err_string = NULL;\
            \
            switch (err) {\
                case GL_INVALID_ENUM:                  err_string = "INVALID ENUM"; break;\
                case GL_INVALID_VALUE:                 err_string = "INVALID_VALUE"; break;\
                case GL_INVALID_OPERATION:             err_string = "INVALID_OPERATION"; break;\
                case GL_OUT_OF_MEMORY:                 err_string = "OUT OF MEMORY"; break; \
                case GL_INVALID_FRAMEBUFFER_OPERATION: err_string = "INVALID FRAMEBUFFER OPERATION"; break;\
            }\
            \
            fprintf(stderr, "OpenGL Error:%s: %s (%d)\n", err_string, file, line);\
        }\
    }

#define GL_TRY(x) x; gl_check_err(__FILE__, __LINE__);
#endif
