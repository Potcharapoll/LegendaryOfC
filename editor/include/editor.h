#ifndef EDITOR_H
#define EDITOR_H
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include "cimgui.h"
#include "cimgui_impl.h"
#include "types.h"

typedef struct {
    ImGuiContext *context; 
    ImGuiIO *io;

    f32 mousex, mousey;
} Editor;

void editor_init(void);
void editor_destroy(void);
void editor_newframe(void);
void editor_render(void);
Editor *get_editor_instance(void);
#endif
