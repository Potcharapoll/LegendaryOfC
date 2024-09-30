#ifdef DEBUG
#ifndef EDITOR_H
#define EDITOR_H
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include <cimgui.h>
#include <cimgui_impl.h>

struct ImGui { 
    ImGuiContext *context; 
    ImGuiIO *io;
};

void editor_init(void);
void editor_destroy(void);
void editor_render(void);
#endif
#endif
