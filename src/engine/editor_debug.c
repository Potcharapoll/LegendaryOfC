#ifdef DEBUG
#include "editor_internal.h"
#include "../core/game.h"
#include "../global.h"

#include <float.h>

void debug_menu(void) {
    igBegin("Debug", NULL, 0);

    igText("GameState");
    char buf[33];
    for (u8 i = 0; i < 32; ++i) {
      buf[i] = ((global.game_state_flag >> i) & 1) ? '1' : '0';
    }
    igText("Flag: %s", buf);
    igText("Act : %hhu", game_get_act());
    igSpacing();

    igText("Scene");
    igText("SceneState: %s", (global.scene->scene_state == MENU) ? "MENU" : (global.scene->scene_state == INTRO) ? "INTRO" : (global.scene->scene_state == INGAME) ? "INGAME" : "ENDGAME");
    igSpacing();

    igText("ChunkId: %d", global.scene->chunk_id);
    if (igButton("Reset Chunk", (ImVec2){0,0})) {
        global.reset_chunk = true;
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Reload Chunk", (ImVec2){0,0})) {
        global.reload_chunk = true;
    }
    igSpacing();

    igText("Fade");
    igText("Faded: %s", (global.scene->faded) ? "True" : "False");
    igText("State: %s", (global.scene->fade_state == FADE_NONE) ? "FADE_NONE" : (global.scene->fade_state == FADE_IN) ? "FADE_IN" : "FADE_OUT");
    igSliderFloat("Alpha", &global.scene->fade_alpha, 0.0f, 1.0f, "%.1f", 0);

    if (igButton("Fade In", (ImVec2){})) {
        global.scene->fade_state = FADE_IN;
        global.scene->fade_alpha = 1.0f;
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Fade Out", (ImVec2){})) {
        global.scene->fade_state = FADE_OUT;
        global.scene->fade_alpha = 0.0f;
    }
    igSpacing();

    igText("Dialog");
    igText("On_Dialog: %s", (global.scene->on_dialog) ? "True" : "False");
    igSpacing();

    igText("Camera");
    static vec2 pos; 
    static vec3 up, front;
    static vec4 color;

    if (igButton("Update Camera", (ImVec2){})) {
        glm_vec2_copy((vec2){global.scene->camera->position.x, global.scene->camera->position.y}, pos);
        glm_vec3_copy((vec3){global.scene->camera->up.x, global.scene->camera->up.y, global.scene->camera->up.z}, up);
        glm_vec3_copy((vec3){global.scene->camera->front.x, global.scene->camera->front.y, global.scene->camera->front.z}, front);
    }
    
    if (igSliderFloat3("up", up, -1000.0f, 1000.0f, "%.3f", 0)) {
        global.scene->camera->up.x = up[0];
        global.scene->camera->up.y = up[1];
        global.scene->camera->up.z = up[2];
    }
    if (igSliderFloat3("front", front, -1000.0f, 1000.0f, "%.3f", 0)) {
        global.scene->camera->front.x = front[0];
        global.scene->camera->front.y = front[1];
        global.scene->camera->front.z = front[2];
    }
    if (igSliderFloat2("position", pos, -1000.0f, 1000.0f, "%.3f", 0)) {
        global.scene->camera->position.x = pos[0];
        global.scene->camera->position.y = pos[1];
    }
    igSpacing();


    if (igColorEdit4("Gradient", color, 0)) {
        global.scene->gradient.x = color[0];
        global.scene->gradient.y = color[1];
        global.scene->gradient.z = color[2];
        global.scene->gradient.w = color[3];
    }
    igSeparator();

    igEnd();
}
#endif
