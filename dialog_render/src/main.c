#include "renderer.h"
#include "dialog.h"
#include "global.h"
#include "window.h"

struct Global global;

struct Dialog *dialog;

void _set_dialog(struct Dialog *dialog) {
    global.DialogState.name = dialog->name;

    global.DialogState.curr_animation_idx = 1;
    global.DialogState.selected_idx       = 0;

    global.DialogState.onAnimation      = false;

    global.DialogState.curr_dialog_node = dialog->DialogList.dialog;
}

void setup(void) {
    global.DialogState.curr_dialog_node   = NULL;
    global.DialogState.onAnimation        = false;
    global.DialogState.selected_idx       = 0;
    global.DialogState.curr_animation_idx = 1; // must be 1 
                                               
    global.fonts.atlas_width  = 140;                                               
    global.fonts.atlas_height = 140;                                               
    global.fonts.glyph_width  = 14;
    global.fonts.glyph_height = 14;
    global.fonts.cols         = 10;
    global.fonts.rows         = 10;

    renderer_init();

    /* dialog = dialog_create("Benny"); dialog_append(dialog, DIALOG_TEXT, &(struct DialogText){.text = "Hi! I'm Benny. Welcome to CVillage, I'm here to guide you look around\\the village."}); */
    /* dialog_append(dialog, DIALOG_TEXT, &(struct DialogText){.text = "This is another dialog text."}); */

    /* dialog_append(dialog, DIALOG_QUESTION, &(struct DialogQuestion){ */
    /*         .question = "Which keyword in C is used to skip the current iteration in the loop?", */
    /*         .answer   = {"goto","continue","break","for"}, */
    /*         .correct_answer_text = "Oh! you're right. Well done.", */
    /*         .wrong_answer_text = "Unfortunally, you're wrong. Try again next time.", */
    /*         .corrent_answer_idx = 1, */
    /* }); */
}

static void normalInput(void) {
    if (glfwGetKey(global.window->handle, GLFW_KEY_Z) == GLFW_PRESS && global.input_delay >= 0.5) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        global.input_delay = 0.0f; 
    }
    else if (glfwGetKey(global.window->handle, GLFW_KEY_X) == GLFW_PRESS && global.input_delay >= 0.5) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        global.input_delay = 0.0f; 
    }

    if (glfwGetKey(global.window->handle, GLFW_KEY_D) == GLFW_PRESS && global.input_delay >= 0.5) {
        /* _set_dialog(dialog); */
        global.input_delay = 0.0f; 
    }
}

void update(void) {

    global.input_delay += global.dt;

    /* if (global.DialogState.curr_dialog_node != NULL) { */
    /*     dialog_input(global.window->handle); */
    /* } */

    normalInput();

    renderer_prepare();

    // render dialog if dialog in DialogState isn't NULL
    /* dialog_render(); */

    renderer_render_quad((vec3s){100, 100, 0}, (vec2s){100,100}, (vec4s){1,0,0,1});
    renderer_render_quad((vec3s){300, 300, 0}, (vec2s){100,100}, (vec4s){1,0,1,1});

    renderer_render_quad_texture((vec3s){500, 500, 0}, (vec2s){140,140}, (vec4s){1,1,1,1}, global.fonts.atlas);

    renderer_render_text((vec3s){100, 500, 0}, (vec4s){1,1,1,1}, "Hello Traveller! I'm Timmy");

    renderer_render();
}

void cleanup(void) {
    /* dialog_delete(dialog); */
    renderer_destroy();
}

int main(void) {
    struct Window window;
    if (window_init(&window, setup, update, cleanup)) {
        global.window = &window;
        window_loop(&window);
    }
    window_destroy(&window);
    return 0;
}
