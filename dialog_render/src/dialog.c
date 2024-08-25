#include "dialog.h"
#include "global.h"
#include "batch_render.h"
#include "renderer.h"
#include "window.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string.h>

#define DIALOG_TEXT_BOX_POSITION (vec3s){100.0f,  25.0f, 0.0f}
#define DIALOG_NAME_BOX_POSITION (vec3s){100.0f, 255.0f, 0.0f}
#define DIALOG_TRIANGLE_POSITION (vec3s){WIDTH - 165.0f, 70.0f, 0.0f}
#define DIALOG_TEXT_POSITION     (vec3s){150.0f, 145.0f, 0.0f}
#define DIALOG_NAME_POSITION     (vec3s){180.0f, 195.0f, 0.0f}
#define DIALOG_QUESTION_POSITION (vec3s){150.0f, 145.0f, 0.0f}
#define DIALOG_ANSWER1_POSITION  (vec3s){240.0f, 100.0f, 0.0f}
#define DIALOG_ANSWER2_POSITION  (vec3s){240.0f,  60.0f, 0.0f}
#define DIALOG_ANSWER3_POSITION  (vec3s){640.0f, 100.0f, 0.0f}
#define DIALOG_ANSWER4_POSITION  (vec3s){640.0f,  60.0f, 0.0f}
#define DIALOG_TEXT_SIZE         (vec2s){WIDTH - 200.0f, 200.0f}
#define DIALOG_NAME_SIZE         (vec2s){200, 50}
#define DIALOG_TRIANGLE_SIZE     (vec2s){15, 15}
#define DIALOG_COLOR             (vec4s){0.5, 0.6, 1.0, 0.8}
#define DIALOG_TEXT_COLOR        (vec4s){1.0, 1.0, 1.0, 0.8}

static vec3s selected_answer_pos[4] = {
    {210.0f, 100.0f, 0.0f},
    {210.0f,  60.0f, 0.0f},
    {610.0f, 100.0f, 0.0f},
    {610.0f,  60.0f, 0.0f},
};

static GLuint _dialog_texture = GL_NONE;
static GLuint _vbo = GL_NONE;
static struct BatchRender *_dialog_batch = NULL;

void dialog_init(void) {

    { 
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex[4]), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    { // load dialog texture
        glGenTextures(1, &_dialog_texture);
        glBindTexture(GL_TEXTURE_2D, _dialog_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_set_flip_vertically_on_load(1);

        s32 width, height, bpp;
        u8 *pixels = stbi_load("images/dialog_box.png", &width, &height, &bpp, 0);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(pixels);
    }

    // initialize dialog batch 
    _dialog_batch = batch_render_init();
    batch_render_append_texture(_dialog_batch, _dialog_texture);
}

void dialog_destroy(void) {
    batch_render_destroy(_dialog_batch);
    glDeleteTextures(1, &_dialog_texture);
}

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

void dialog_input(void) {
    if (glfwGetKey(global.window->handle, GLFW_KEY_SPACE) == GLFW_PRESS && global.input_delay >= 0.2 && !global.DialogState.onAnimation) {

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

    if (glfwGetKey(global.window->handle, GLFW_KEY_S) == GLFW_PRESS && global.input_delay >= 0.2 
    && !global.DialogState.onAnimation && global.DialogState.curr_dialog_node->type == DIALOG_QUESTION) {
        global.DialogState.selected_idx = (global.DialogState.selected_idx + 1) % 4;
        global.input_delay = 0.0f; 
    }
}

void dialog_render(void) {
    if (!global.DialogState.curr_dialog_node) return;

    {
        u32 tex_slot = batch_render_get_texture_slot(_dialog_batch, _dialog_texture);

        vec3s position = DIALOG_TEXT_POSITION;
        vec2s size     = DIALOG_TEXT_SIZE;
        vec4s color    = DIALOG_TEXT_COLOR;
        struct Vertex vertices[4];

        vertices[0] = (struct Vertex){ 
            .position = { .x = position.x, .y = position.y, .z = position.z}, 
            .color = color, 
            .tex_coord = {0,0}, 
            .tex_slot = tex_slot 
        };
        vertices[1] = (struct Vertex){ 
            .position = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
            .color = color, 
            .tex_coord = {1,0}, 
            .tex_slot = tex_slot 
        };
        vertices[2] = (struct Vertex){ 
            .position = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
            .color = color, 
            .tex_coord = {1,1}, 
            .tex_slot = tex_slot 
        };
        vertices[3] = (struct Vertex){ 
            .position = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
            .color = color, 
            .tex_coord = {0,1}, 
            .tex_slot = tex_slot 
        };

        glBindVertexArray(_dialog_batch->vao);

        glBindBuffer(GL_ARRAY_BUFFER, _dialog_batch->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }


    // render dialog box
    batch_render_append_quad_texture(_dialog_batch, DIALOG_TEXT_BOX_POSITION, DIALOG_TEXT_SIZE, DIALOG_COLOR, _dialog_texture);

    // render name text
    renderer_render_text(DIALOG_NAME_POSITION, (vec4s){0.8, 0.8, 0.8, 1.0}, global.DialogState.name);

    // render dialog text
    if (global.DialogState.curr_dialog_node == NULL) {
        global.DialogState.curr_dialog_node = NULL;
        global.DialogState.selected_idx     = 0;
        return;
    }

    switch (global.DialogState.curr_dialog_node->type) {
        case DIALOG_TEXT: 
            ;
            struct DialogText *text = global.DialogState.curr_dialog_node->dialog;
            renderer_render_dialog_text_animation(DIALOG_TEXT_POSITION, DIALOG_TEXT_COLOR, text->text); 
            break;

        case DIALOG_QUESTION:
            ;
            struct DialogQuestion *question = global.DialogState.curr_dialog_node->dialog;
            renderer_render_dialog_text_animation(DIALOG_QUESTION_POSITION, DIALOG_TEXT_COLOR, question->question);  

            renderer_render_text(DIALOG_ANSWER1_POSITION, DIALOG_TEXT_COLOR, question->answer[0]); 
            renderer_render_text(DIALOG_ANSWER2_POSITION, DIALOG_TEXT_COLOR, question->answer[1]); 
            renderer_render_text(DIALOG_ANSWER3_POSITION, DIALOG_TEXT_COLOR, question->answer[2]); 
            renderer_render_text(DIALOG_ANSWER4_POSITION, DIALOG_TEXT_COLOR, question->answer[3]); 

            renderer_render_quad(selected_answer_pos[global.DialogState.selected_idx], (vec2s){15,15}, DIALOG_TEXT_COLOR);
            break;
    }

    batch_render_render(_dialog_batch);

    { // render triangle
        static f32 animation = 15.0f;
        static vec3s tri_pos = DIALOG_TRIANGLE_POSITION; 


        // render

        if (tri_pos.y >= 80 || tri_pos.y <= 70) { animation *= -1; } 
        else { tri_pos = DIALOG_TRIANGLE_POSITION; }

        tri_pos.y += animation * global.dt;
    }

}

// OLD VERSION
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
