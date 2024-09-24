#include "editor_internal.h"
#include "../global.h"

#include <string.h>

// Make the collider can make DIALOG and TELEPORTER

#define GET_LAYER_STRING(layer, buf) \
    switch (layer) { \
        case COLLISION_LAYER_NONE:        strcpy(buf, "NONE"); break; \
        case COLLISION_LAYER_SOLID:       strcpy(buf, "SOLID"); break; \
        case COLLISION_LAYER_PLAYER:      strcpy(buf, "PLAYER"); break; \
        case COLLISION_LAYER_DIALOG:      strcpy(buf, "DIALOG"); break; \
        case COLLISION_LAYER_TELEPORTER:  strcpy(buf, "TELEPORTER"); break; \
        default:                          strcpy(buf, "UNKNOWN"); break; \
    }

void collider_menu(void) {
    static array_list *list;
    static vec2 size = {0};
    static char mask_buf[50];
    static char flag_buf[50];

    list = physics_get_static_body_list();

    igBegin("Collider List", NULL, 0);
    igCheckbox("Show Collider", &global.toggle_show_collider);
    igSameLine(0.0f, 10.0f);
    igCheckbox("Disable Collision", &global.toggle_collision);

    if (igButton("Delete All", (ImVec2){0,0})) {
        list->len = 0;
    }
    igSeparator();

    size[0] = global.end_point[0] - global.start_point[0];
    size[1] = global.end_point[1] - global.start_point[1];

    igSeparator();
    igText("Size     : %.2f, %.2f", size[0], size[1]);
    igText("HalfSize : %.2f, %.2f", (global.end_point[0] - global.start_point[0]) * 0.5, (global.end_point[1] - global.start_point[1]) * 0.5);
    if (igButton("Floor", (ImVec2){125,0})) {
        global.start_point[0] = floor(global.start_point[0]);
        global.start_point[1] = floor(global.start_point[1]);
        global.end_point[0] = floor(global.end_point[0]);
        global.end_point[1] = floor(global.end_point[1]);
        size[0] = floor(size[0]);
        size[1] = floor(size[1]);
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Swap", (ImVec2){125,0})) {
        vec2 temp;
        glm_vec2_copy(global.start_point, temp);
        glm_vec2_copy(global.end_point, global.start_point); 
        glm_vec2_copy(global.end_point, temp);
    }


    if (igButton("-X##0", (ImVec2){})) {
        global.start_point[0] -= 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    if (igButton("+X##0", (ImVec2){})) {
        global.start_point[0] += 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    if (igButton("-Y##0", (ImVec2){})) {
        global.start_point[1] -= 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    if (igButton("+Y##0", (ImVec2){})) {
        global.start_point[1] += 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    igSetNextItemWidth(150.0f);
    igInputFloat2("Start Point", global.start_point, "%.2f", 0);

    if (igButton("-X##1", (ImVec2){})) {
        global.end_point[0] -= 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    if (igButton("+X##1", (ImVec2){})) {
        global.end_point[0] += 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    if (igButton("-Y##1", (ImVec2){})) {
        global.end_point[1] -= 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    if (igButton("+Y##1", (ImVec2){})) {
        global.end_point[1] += 1.0f;
    }   
    igSameLine(0.0f, 5.0f);
    igSetNextItemWidth(150.0f);
    igInputFloat2("End Point", global.end_point, "%.2f", 0);

    static char *types[] = {"SOLID", "TELEPORTER", "DIALOG"};
    static u8 selected_idx = 0;

    igSetNextItemWidth(260.0f);
    if (igBeginCombo("Collider Type", types[selected_idx], 0)) {
        for (u8 i = 0; i < 3; ++i) {
            b8 selected = (i == selected_idx) ? true : false;
            if (igSelectable_Bool(types[i], selected, 0, (const ImVec2){})) {
                selected_idx = i;
            }
        }
        igEndCombo();
    }

    if (selected_idx == 1) {
        static u8    selected_chunk_idx = 0;
        static char *chunk_name[] = {
            "CHUNK_SPAWN"            ,
            "CHUNK_VILLAGE_ENTRANCE" ,
            "CHUNK_VILLAGE_LEFT"     ,
            "CHUNK_VILLAGE_RIGHT"    ,
            "CHUNK_VILLAGE_TOP"      ,
            "CHUNK_VILLAGE_TOP_END"  ,
            "CHUNK_VILLAGE_TOP_LEFT" ,
            "CHUNK_VILLAGE_TOP_RIGHT",
            "CHUNK_VILLAGE_TUNNEL"   ,
            "CHUNK_INSIDE_LIBRARY"   ,
        };

        igSetNextItemWidth(260.0f);
        if (igBeginCombo("ChunkId", chunk_name[selected_chunk_idx], 0)) {
            for (u8 i = 0; i < CHUNK_LAST; ++i) {
                b8 chunk_selected = (i == selected_chunk_idx) ? true : false;
                if (igSelectable_Bool(chunk_name[i], chunk_selected, 0, (const ImVec2){})) {
                    selected_chunk_idx = i;
                }
            }
            igEndCombo();
        }

    }
    else if (selected_idx == 2) {
        static s32 dialogid = 0;

        igSetNextItemWidth(260.0f);
        if (igInputInt("DialogId", &dialogid, 1, 1, 0)) {

            if (dialogid < 0) {
                dialogid = 0;
            }

            dialogid = dialogid % 2;
        }
    }

    if (igButton("New Collider", (ImVec2){260,24})) {
        u8 _mask = COLLISION_LAYER_PLAYER;
        u8 _flag = (selected_idx == 0 ) ? COLLISION_LAYER_SOLID : (selected_idx == 1) ? COLLISION_LAYER_DIALOG : COLLISION_LAYER_TELEPORTER;

        vec2s size = { fabsf(global.end_point[0] - global.start_point[0]), fabsf(global.end_point[1] - global.start_point[1]), };

        switch (selected_idx) {
            case 0:
                physics_static_body_create((vec2s){global.start_point[0], global.start_point[1]}, size, _mask, _flag, NULL);
                break;
            default:
                physics_static_body_create((vec2s){global.start_point[0], global.start_point[1]}, size, _mask, _flag, global.collision_callback);
                break;
        }
    }
    igSeparator();

    for (u32 i = 0; i < list->len; ++i) {
        Static_Body *body = physics_static_body_get(i);

        GET_LAYER_STRING(body->collision_mask, mask_buf);
        GET_LAYER_STRING(body->collision_flag, flag_buf);

        igText("Id %i\n"
               "\tCenter   { %.2f, %2.f }\n"
               "\tHalfSize { %.2f, %.2f }\n"
               "\tMASK     %s\n"
               "\tFLAG     %s\n",
                i, body->aabb.center.x, body->aabb.center.y, body->aabb.half_size.x, body->aabb.half_size.y, mask_buf, flag_buf);
    }

    igEnd();
}
