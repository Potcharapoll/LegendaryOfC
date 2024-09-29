#ifndef LOGGER_H
#define LOGGER_H

// Disable debug and trace when building release version

typedef enum log_level {
    LOG_LEVEL_FETAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,
} log_level;

void assetion_failure(const char* expression, const char *msg, const char *file, int line);
void log_msg(log_level level, char *fmt, ...);

#define LOG_FETAL(fmt, ...) log_msg(LOG_LEVEL_FETAL, fmt, ##__VA_ARGS__); exit(1);
#define LOG_ERROR(fmt, ...) log_msg(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__);
#define LOG_WARN(fmt, ...) log_msg(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__);
#define LOG_INFO(fmt, ...) log_msg(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__);
#define LOG_DEBUG(fmt, ...) log_msg(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__);
#define LOG_TRACE(fmt, ...) log_msg(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__);
#define ASSERT(expression, msg, file, line) if(!(expression)) assetion_failure(#expression, msg, file, line);

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
            LOG_ERROR("OpenGL error: %s at %s:%d", err_string, file, line);\
        }\
    }

#define GL_TRY(x) x; gl_check_err(__FILE__, __LINE__);
#endif
