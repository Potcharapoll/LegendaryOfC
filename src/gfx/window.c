#include "window.h"
#include "../util/log.h"
#include "../global.h"
#include "../defs.h"

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int err, const char *dest) {
    printf("GLFW Error Callback %d: %s\n", err, dest);
}

b8 window_init(struct Window *self, wfunc init, wfunc update, wfunc cleanup) {
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) {
        abort();
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    self->init    = init;
    self->update  = update;
    self->cleanup = cleanup;
    self->width   = WIDTH;
    self->height  = HEIGHT;
    self->handle  = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
    if(!self->handle) return false;
    
    glfwMakeContextCurrent(self->handle);
    glfwSwapInterval(1);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    printf("GLFW Version: %s\n", glfwGetVersionString());
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    return true;
}

void window_loop(struct Window *self) {
    self->init();
    f32 last_frame = glfwGetTime();

    glViewport(0, 0, WIDTH, HEIGHT);
    while (!glfwWindowShouldClose(self->handle)) {
        glfwGetCursorPos(self->handle, &self->mouse.xpos, &self->mouse.ypos);
        glfwGetWindowSize(self->handle, &self->width, &self->height);

        glViewport(0, 0, self->width, self->height);

        // normalize mouse position
        self->mouse.normalx = (self->mouse.xpos / self->width) * 2.0f - 1.0f;
        self->mouse.normaly = ((self->height - self->mouse.ypos) / self->height) * 2.0f - 1.0f;
        
        f32 current_frame = glfwGetTime();
        global.dt = (current_frame - last_frame);
        last_frame = current_frame;

        glfwPollEvents();
        self->update();
        glfwSwapBuffers(self->handle);
    }
}

void window_destroy(struct Window *self) {
    self->cleanup();
    glfwDestroyWindow(self->handle);
    glfwTerminate();
    LOG_DEBUG("Window destroyed");
}

