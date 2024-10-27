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
    igText("Act : %hhu", (game_get_act() + 1));
    igText("Flag: %s", buf);
    igSpacing();

    igText("Scene");
    igText("Chunk: %d", global.scene->chunk_id);
    igText("SceneState: %s", (global.scene->scene_state == SCENE_MENU) ? "MENU" : (global.scene->scene_state == SCENE_INTRO) ? "INTRO" : (global.scene->scene_state == SCENE_INGAME) ? "INGAME" : "ENDGAME");
    igSpacing();

    if (igButton("Reset Chunk", (ImVec2){0,0})) {
        global.reset_chunk = true;
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Reload Chunk", (ImVec2){0,0})) {
        global.reload_chunk = true;
    }
    igSpacing();

    igText("Fade");
    igText("State : %s", (global.scene->fade_state == FADE_NONE) ? "FADE_NONE" : (global.scene->fade_state == FADE_IN) ? "FADE_IN" : "FADE_OUT");
    igText("Faded : %s", (global.scene->faded) ? "True" : "False");
    igText("Fading: %s", (global.scene->fading) ? "True" : "False");
    igSliderFloat("Alpha", &global.scene->fade_alpha, 0.0f, 1.0f, "%.1f", 0);
    igSpacing();

    igText("Dialog");
    igText("On_Dialog: %s", (global.scene->on_dialog) ? "True" : "False");
    igSpacing();

    igText("Camera");
    static vec2 pos; 
    static vec3 up, front;

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
    igSeparator();

    igEnd();
}
#endif
