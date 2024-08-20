#include "dialog.h"
#include "global.h"

#include <string.h>
#include <cglm/struct.h>

/* static vec3s selected_answer_pos[4] = { */
/*     {210.0f, 100.0f, 0.0f}, */
/*     {210.0f,  60.0f, 0.0f}, */
/*     {610.0f, 100.0f, 0.0f}, */
/*     {610.0f,  60.0f, 0.0f}, */
/* }; */

struct Dialog* dialog_create(char *name) {
    struct Dialog *d = malloc(sizeof(*d));

    u32 len = strlen(name);
    d->name = malloc(len + 1);
    strcpy(d->name, name);

    d->DialogList.dialog = NULL;
    d->DialogList.count  = 0;

    d->hidden_name = false;
    return d;
}

void dialog_delete(struct Dialog *dialog) {
    struct DialogNode *node = dialog->DialogList.dialog;
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

    if (dialog->DialogList.dialog == NULL) {
        dialog->DialogList.dialog = node;
        dialog->DialogList.count++;
        return;
    }

    struct DialogNode *curr = dialog->DialogList.dialog;
    while (curr->next != NULL) {
        curr = curr->next;
    } 

    curr->next = node;
    dialog->DialogList.count++;
}

void dialog_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && global.input_delay >= 0.2 && !global.DialogState.onAnimation) {

        if(global.DialogState.curr_dialog_node->type == DIALOG_QUESTION){
                struct DialogQuestion *qt = global.DialogState.curr_dialog_node->dialog;
                struct DialogNode *node = malloc(sizeof(*node));
                node->type      = DIALOG_TEXT;
                node->dialog    = malloc(sizeof(struct DialogText)); 
                node->next      = NULL;

                if (global.DialogState.selected_idx == qt->corrent_answer_idx) {
                    memcpy(node->dialog, &(struct DialogText){.text = qt->correct_answer_text }, sizeof(struct DialogText));
                }
                else {
                    memcpy(node->dialog, &(struct DialogText){.text = qt->wrong_answer_text }, sizeof(struct DialogText));
                }

                global.DialogState.curr_dialog_node->next = node;
        }

        global.DialogState.curr_dialog_node   = global.DialogState.curr_dialog_node->next;
        global.DialogState.curr_animation_idx = 1;
        global.input_delay = 0.0f; 
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && global.input_delay >= 0.2 
    && !global.DialogState.onAnimation && global.DialogState.curr_dialog_node->type == DIALOG_QUESTION) {
        global.DialogState.selected_idx = (global.DialogState.selected_idx + 1) % 4;
        global.input_delay = 0.0f; 
    }
}

// we will use the global state
/* void dialog_render(void) { */
/*     if (!global.DialogState.curr_dialog_node) return; */

/*     glEnable(GL_BLEND); */
/*     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */

/*     // render dialog box */
/*     renderer_render_quad_texture(DIALOG_TEXT_BOX_POSITION, DIALOG_TEXT_SIZE, DIALOG_COLOR, _dialog_texture); */

/*     { // render triangle */
/*         static f32 animation = 15.0f; */
/*         static vec3s tri_pos = DIALOG_TRIANGLE_POSITION; */ 

/*         renderer_render_triangle(tri_pos, DIALOG_TRIANGLE_SIZE, DIALOG_TEXT_COLOR); */
/*         if (tri_pos.y >= 80 || tri_pos.y <= 70) { animation *= -1; } */ 
/*         else { tri_pos = DIALOG_TRIANGLE_POSITION; } */

/*         tri_pos.y += animation * global.dt; */
/*     } */

/*     // render name text */
/*     renderer_render_text(DIALOG_NAME_POSITION, (vec4s){0.8, 0.8, 0.8, 1.0}, global.DialogState.name); */

/*     // render dialog text */
/*     if (global.DialogState.curr_dialog_node == NULL) { */
/*         global.DialogState.curr_dialog_node = NULL; */
/*         global.DialogState.selected_idx     = 0; */
/*         return; */
/*     } */

/*     switch (global.DialogState.curr_dialog_node->type) { */
/*         case DIALOG_TEXT: */ 
/*             ; */
/*             struct DialogText *text = global.DialogState.curr_dialog_node->dialog; */
/*             renderer_render_dialog_text_animation(DIALOG_TEXT_POSITION, DIALOG_TEXT_COLOR, text->text); */ 
/*             break; */

/*         case DIALOG_QUESTION: */
/*             ; */
/*             struct DialogQuestion *question = global.DialogState.curr_dialog_node->dialog; */
/*             renderer_render_dialog_text_animation(DIALOG_QUESTION_POSITION, DIALOG_TEXT_COLOR, question->question); */  

/*             renderer_render_text(DIALOG_ANSWER1_POSITION, DIALOG_TEXT_COLOR, question->answer[0]); */ 
/*             renderer_render_text(DIALOG_ANSWER2_POSITION, DIALOG_TEXT_COLOR, question->answer[1]); */ 
/*             renderer_render_text(DIALOG_ANSWER3_POSITION, DIALOG_TEXT_COLOR, question->answer[2]); */ 
/*             renderer_render_text(DIALOG_ANSWER4_POSITION, DIALOG_TEXT_COLOR, question->answer[3]); */ 

/*             renderer_render_quad(selected_answer_pos[global.DialogState.selected_idx], (vec2s){15,15}, DIALOG_TEXT_COLOR); */
/*             break; */
/*     } */
/* } */
