#include "../global.h"
#include "../defs.h"

#include "dialog.h"
#include "asset_manager.h"
/* #include "renderer.h" */

#include <string.h>

typedef enum {
    DIALOG_POSITION_TITLE,
    DIALOG_POSITION_TEXT,
    DIALOG_POSITION_ANSWER1,
    DIALOG_POSITION_ANSWER2,
    DIALOG_POSITION_ANSWER3,
    DIALOG_POSITION_ANSWER4,
    DIALOG_POSITION_SELECT,

    DIALOG_POSITION_LAST,
} DialogRenderPosition;

static vec3s dialog_render_position[DIALOG_POSITION_LAST] = {
    { 14,       55,      0.0f},
    { 15,       40,      0.0f},
    { 15 + 50,  40 - 18, 0.0f},
    { 15 + 50,  40 - 38, 0.0f},
    { 15 + 150, 40 - 18, 0.0f},
    { 15 + 150, 40 - 38, 0.0f},
    { 10,       -6,      0.0f},
};

static b8 next_dialog   = true;
static b8 animation_end = false;

/* static void dialog_render_text(char *text, vec2s size, vec3s pos, vec4s color) { */
/*     f32 tex_coord[4]; */

/*     struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT); */
/*     vec2s cell_size = {(f32)sp->stride / sp->texture.size.x, (f32)sp->stride / sp->texture.size.y}; */

/*     for (u32 idx = 0; idx < strlen(text); ++idx) { */
/*         ivec2s char_coord = get_char_coord(text[idx]); */

/*         tex_coord[0] = cell_size.x * char_coord.x; */
/*         tex_coord[1] = cell_size.x * char_coord.x + cell_size.x; */
/*         tex_coord[2] = cell_size.y * char_coord.y; */
/*         tex_coord[3] = cell_size.y * char_coord.y + cell_size.y; */

/*         /1* renderer_append_quad_texture(LAYER_DIALOG, pos, size, color, sp->texture, tex_coord); *1/ */
/*         pos.x += (size.x * 0.5); */
/*     } */
/* } */

/* static void dialog_render_text_animation(char *text, vec2s size, vec3s pos, vec4s color) { */
/*     static u32 current_rendered_idx = 0; */
/*     static u32 max_rendered_idx     = 0; */


/*     if (next_dialog) { */
/*         current_rendered_idx = 0; */
/*         max_rendered_idx     = strlen(text); */
/*         next_dialog          = false; */
/*         animation_end        = false; */
/*     } */
    

/*     f32 tex_coord[4]; */

/*     struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT); */
/*     vec2s cell_size = {(f32)sp->stride / sp->texture.size.x, (f32)sp->stride / sp->texture.size.y}; */

/*     for (u32 idx = 0; idx < current_rendered_idx; ++idx) { */
/*         ivec2s char_coord = get_char_coord(text[idx]); */

/*         tex_coord[0] = cell_size.x * char_coord.x; */
/*         tex_coord[1] = cell_size.x * char_coord.x + cell_size.x; */
/*         tex_coord[2] = cell_size.y * char_coord.y; */
/*         tex_coord[3] = cell_size.y * char_coord.y + cell_size.y; */

/*         /1* renderer_append_quad_texture(LAYER_DIALOG, pos, size, color, sp->texture, tex_coord); *1/ */
/*         pos.x += (size.x * 0.5); */
/*     } */

/*     if (current_rendered_idx >= max_rendered_idx) { */
/*         current_rendered_idx++; */
/*     } else { */
/*         animation_end = true; */
/*     } */
/* } */

struct Dialog* dialog_create(char *name) {
    u32 len = strlen(name);

    struct Dialog *d = malloc(sizeof(*d));
    d->contents = NULL;
    d->name = malloc(len + 1);
    strcpy(d->name, name);

    return d;
}

void dialog_delete(struct Dialog *dialog) {
    struct DialogNode *node = dialog->contents;
    while (node != NULL) {
        struct DialogNode *tmp = node;
        node = node->next;

        free(tmp->dialog);
        free(tmp);
    }

    free(dialog->name);
    free(dialog);
}

void dialog_append(struct Dialog *dialog, enum DialogType type, void *data) {
    u32 dialog_size = (type == DIALOG_TEXT) ? sizeof(struct DialogText) : sizeof(struct DialogQuestion);

    struct DialogNode *node = malloc(sizeof(*node));

    node->type      = type;
    node->dialog    = malloc(dialog_size); 
    node->next      = NULL;
    memcpy(node->dialog, data, dialog_size);

    if (dialog->contents == NULL) {
        dialog->contents = node;
        return;
    }

    struct DialogNode *curr = dialog->contents;
    while (curr->next != NULL) {
        curr = curr->next;
    } 

    curr->next = node;
}

void dialog_input(void) {
    static f32 delay = 0.0f;

    delay += global.dt;

    if (animation_end && delay >= 0.2) {
        if(window_get_key(global.window, GLFW_KEY_SPACE)) {
            if(global.DialogState.curr_dialog_node->type == DIALOG_QUESTION){
                struct DialogQuestion *qt = global.DialogState.curr_dialog_node->dialog;

                struct DialogNode *node = malloc(sizeof(*node));
                node->type      = DIALOG_TEXT;
                node->dialog    = malloc(sizeof(struct DialogText)); 
                node->next      = NULL;

                if (global.DialogState.selected_answer == qt->correct_answer_idx) {
                    memcpy(node->dialog, &(struct DialogText){.text = qt->correct_answer_text }, sizeof(struct DialogText));
                }
                else {
                    memcpy(node->dialog, &(struct DialogText){.text = qt->wrong_answer_text }, sizeof(struct DialogText));
                }

                global.DialogState.curr_dialog_node->next = node;
            }

            global.DialogState.curr_dialog_node = global.DialogState.curr_dialog_node->next;
            next_dialog = true;
            delay = 0.0f;
        }

        if (window_get_key(global.window, GLFW_KEY_S) && global.DialogState.curr_dialog_node->type == DIALOG_QUESTION) {
            global.DialogState.selected_answer = (global.DialogState.selected_answer + 1) % 4;
            delay = 0.0f;
        }
    }
}

void dialog_render(void) {
    /* vec3s camera_pos = { global.camera->position.x, global.camera->position.y, 0.0f}; */
    /* vec3s pos        = {0}; */
    /* renderer_append_quad(LAYER_DIALOG, camera_pos, DIALOG_FRAME_SIZE, DIALOG_FRAME_COLOR); */

    /* pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TITLE]); */
    /* dialog_render_text(global.DialogState.name, (vec2s){10,11}, pos, BLUE); */
    /* dialog_render_text(global.DialogState.name, (vec2s){10,10}, pos, DIALOG_TEXT_COLOR); */

    /* pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TEXT]); */
    /* if (global.DialogState.curr_dialog_node->type == DIALOG_QUESTION) { */
    /*     struct DialogQuestion *content = global.DialogState.curr_dialog_node->dialog; */
    /*     dialog_render_text_animation(content->question, DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */

    /*     u8 selected_answer = DIALOG_POSITION_LAST - 5 + global.DialogState.selected_answer; */
    /*     if (animation_end) { */
    /*         pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER1]); */
    /*         dialog_render_text(content->answer[0], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */

    /*         pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER2]); */
    /*         dialog_render_text(content->answer[1], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */

    /*         pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER3]); */
    /*         dialog_render_text(content->answer[2], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */

    /*         pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER4]); */
    /*         dialog_render_text(content->answer[3], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */

    /*         pos = glms_vec3_add(camera_pos, glms_vec3_sub(dialog_render_position[selected_answer], dialog_render_position[DIALOG_POSITION_SELECT])); */
    /*         renderer_append_quad(LAYER_DIALOG, pos, DIALOG_SELECT_SIZE, DIALOG_TEXT_COLOR); */
    /*     } */
    /* } */
    /* else { */
    /*     struct DialogText *content = global.DialogState.curr_dialog_node->dialog; */
    /*     dialog_render_text_animation(content->text, DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */
    /* } */
}

