#include "editor_internal.h"
#include "../core/renderer.h"
#include "../global.h"

void debug_menu(void) {
    igBegin("Debug", NULL, 0);

    igText("Chunk");
    if (igButton("Reset Chunk", (ImVec2){0,0})) {
        renderer_reset_chunk();
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Reload Chunk", (ImVec2){0,0})) {
        renderer_reload_chunk();
    }
    igSeparator();

    igText("Fadeing");
    igText("State: %s", (global.FadeState.state == FADE_NONE) ? "FADE_NONE" : (global.FadeState.state == FADE_IN) ? "FADE_IN" : "FADE_OUT");
    igSliderFloat("Alpha", &global.FadeState.alpha, 0.0f, 1.0f, "%.1f", 0);

    if (igButton("Fade In", (ImVec2){})) {
        global.FadeState.state = FADE_IN;
        global.FadeState.alpha = 1.0f;
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Fade Out", (ImVec2){})) {
        global.FadeState.state = FADE_OUT;
        global.FadeState.alpha = 0.0f;
    }
    igSeparator();

    static vec4 color;
    if (igColorEdit4("Gradient", color, 0)) {
        global.gradient.x = color[0];
        global.gradient.y = color[1];
        global.gradient.z = color[2];
        global.gradient.w = color[3];
    }
    igEnd();
}
