#ifndef WINDOW_H
#define WINDOW_H
#include "../util/types.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef void (*wfunc)(void);

struct Window {
    GLFWwindow *handle;
    wfunc init, update, cleanup;

    f32 delta_time;
    s32 width, height;

    struct {
        f64 xpos, ypos;
        f64 normalx, normaly;
        f64 orthox, orthoy;
    } mouse;
};

b8   window_init(struct Window *self, wfunc init, wfunc update, wfunc cleanup);
void window_loop(struct Window *self);
void window_destroy(struct Window *self);
#endif
