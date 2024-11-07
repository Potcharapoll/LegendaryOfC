#include "window.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"
#include "GLFW/glfw3.h"

#include <assert.h>

// temp
static b8 should_close = false;

static void error_callback(int err, const char *dest) {
    printf("GLFW Error Callback %d: %s\n", err, dest);
}

void window_init(struct Window *self, wfunc init, wfunc update, wfunc cleanup) {
    glfwSetErrorCallback(error_callback);

    ASSERT(glfwInit() != GLFW_FALSE, "Failed to initialize GLFW", __FILE__, __LINE__);

    LOG_TRACE("Successfully initialized GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    self->init    = init;
    self->update  = update;
    self->cleanup = cleanup;
    self->width   = WIDTH;
    self->height  = HEIGHT;
    self->handle  = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
    ASSERT(self->handle != NULL, "Failed to create GLFWwindow", __FILE__, __LINE__);
    LOG_TRACE("Successfully initialized GLFWwindow");
    
    glfwSetWindowSizeLimits(self->handle, WIDTH, HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(self->handle);
    glfwSwapInterval(1);
    glfwSetInputMode(self->handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize OpenGL", __FILE__, __LINE__); 
    LOG_TRACE("Successfully initialized OpenGL");

    LOG_INFO("GLFW Version: %s", glfwGetVersionString());
    LOG_INFO("OpenGL Version: %s", glGetString(GL_VERSION));
}

void window_loop(struct Window *self) {
    self->init();

    f32 last_frame = glfwGetTime();
    f32 current_frame;
    char title[100];

    while (!glfwWindowShouldClose(self->handle) && !should_close) {
        glfwGetCursorPos(self->handle, &self->mouse.xpos, &self->mouse.ypos);
        glfwGetWindowSize(self->handle, &self->width, &self->height);
        glViewport(0, 0, self->width, self->height);

#ifdef DEBUG
        snprintf(title, sizeof(title), "%s [Debug] (FPS:%.5f/%.5fms)", TITLE, 1 / global.dt, global.dt * 1000);
#else
        snprintf(title, sizeof(title), "%s [Release] (FPS:%.5f/%.5fms)", TITLE, 1 / global.dt, global.dt * 1000);
#endif
        
        glfwSetWindowTitle(self->handle, title);

        // normalize mouse position
        self->mouse.ypos    = self->height - self->mouse.ypos;
        self->mouse.normalx = (self->mouse.xpos / self->width) * 2.0 - 1.0;
        self->mouse.normaly = (self->mouse.ypos / self->height) * 2.0 - 1.0;
        
        current_frame = glfwGetTime();
        global.dt     = (current_frame - last_frame);
        last_frame    = current_frame;

        self->update();
        glfwPollEvents();
        glfwSwapBuffers(self->handle);
    }
}

void window_destroy(struct Window *self) {
    self->cleanup();

    glfwDestroyWindow(self->handle);
    glfwTerminate();
    LOG_TRACE("Window destroyed");
}


b8   window_get_key(struct Window *self, int key) {
    return (glfwGetKey(self->handle, key) == GLFW_PRESS);
}

b8 window_get_mouse_button(struct Window *self, int button) {
    return (glfwGetMouseButton(self->handle, button) == GLFW_PRESS);
}

void window_trigger_close(void) {
  should_close = true;
}
