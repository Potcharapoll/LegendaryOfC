#include "dialog.h"

#include <string.h>
#include <stdlib.h>

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

Dialog* dialog_create(void) {
    struct Dialog *d = malloc(sizeof(*d));
    d->length = 0;
    d->contents = NULL;
    return d;
}

void dialog_delete(Dialog *dialog) {
    DialogNode *node = dialog->contents;
    while (node != NULL) {
        DialogNode *tmp = node;
        node = node->next;

        free(tmp->name);
        free(tmp->dialog);
        free(tmp);
    }

    free(dialog);
}

void dialog_append(Dialog *dialog, char *name, enum DialogType type, void *data) {
    u32 dialog_size = (type == DIALOG_TYPE_TEXT) ? sizeof(DialogText) : sizeof(DialogQuestion);

    DialogNode *node = malloc(sizeof(*node));

    node->type   = type;
    node->dialog = malloc(dialog_size); 
    node->name   = malloc(strlen(name) + 1);
    node->next   = NULL;
    memcpy(node->dialog, data, dialog_size);
    strcpy(node->name, name);

    if (dialog->contents == NULL) {
        dialog->contents = node;
        return;
    }

    DialogNode *curr = dialog->contents;
    while (curr->next != NULL) {
        curr = curr->next;
    } 

    curr->next = node;
    dialog->length++;
}

void dialog_input(void) {
    /* if (animation_end && delay >= 0.2) { */
    /*     if(window_get_key(global.window, GLFW_KEY_SPACE)) { */
    /*         if(global.DialogState.curr_dialog_node->type == DIALOG_QUESTION){ */
    /*             struct DialogQuestion *qt = global.DialogState.curr_dialog_node->dialog; */

    /*             struct DialogNode *node = malloc(sizeof(*node)); */
    /*             node->type      = DIALOG_TEXT; */
    /*             node->dialog    = malloc(sizeof(struct DialogText)); */ 
    /*             node->next      = NULL; */

    /*             if (global.DialogState.selected_answer == qt->correct_answer_idx) { */
    /*                 memcpy(node->dialog, &(struct DialogText){.text = qt->correct_answer_text }, sizeof(struct DialogText)); */
    /*             } */
    /*             else { */
    /*                 memcpy(node->dialog, &(struct DialogText){.text = qt->wrong_answer_text }, sizeof(struct DialogText)); */
    /*             } */

    /*             global.DialogState.curr_dialog_node->next = node; */
    /*         } */

    /*         global.DialogState.curr_dialog_node = global.DialogState.curr_dialog_node->next; */
    /*         next_dialog = true; */
    /*         delay = 0.0f; */
    /*     } */

    /*     if (window_get_key(global.window, GLFW_KEY_S) && global.DialogState.curr_dialog_node->type == DIALOG_QUESTION) { */
    /*         global.DialogState.selected_answer = (global.DialogState.selected_answer + 1) % 4; */
    /*         delay = 0.0f; */
    /*     } */
    /* } */
}

