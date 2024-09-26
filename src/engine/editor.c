#include "editor.h"
#include "editor_internal.h"
#include "../global.h"
#include "logger.h"

// NOTE: Use Multi-Viewport cuase an error from GLFW 
// GLFW Error Callback 65548: Wayland: The platform does not provide the window position

void editor_init(void) {
    struct ImGui *editor = malloc(sizeof(*editor));
    ASSERT(editor != NULL, "Failed to allocate memory for editor", __FILE__, __LINE__);

    editor->context = igCreateContext(NULL);
    editor->io      = igGetIO();

    /* editor->io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; */

    ImGui_ImplGlfw_InitForOpenGL(global.window->handle, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    igStyleColorsDark(NULL);

    global.editor = editor;

    LOG_TRACE("Editor: Successfully initialized editor");
}

void editor_destroy(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(global.editor->context);

    free(global.editor);

    LOG_TRACE("Editor: Successfully destroyed editor");
}

void editor_render(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    collider_menu();     
    debug_menu();

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    /* if (global.editor->io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) { */
    /*   GLFWwindow *backup_current_window = glfwGetCurrentContext(); */
    /*   igUpdatePlatformWindows(); */
    /*   igRenderPlatformWindowsDefault(NULL, NULL); */
    /*   glfwMakeContextCurrent(backup_current_window); */
    /* } */
}
