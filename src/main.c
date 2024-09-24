#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/renderer.h"
#include "gfx/window.h"

#define DEBUG_INPUT
/* #define TEST_DIALOG */

#include "global.h"
#include "defs.h"

//   ~80%
//   Custom Collision         -- ON GOING --
//   Inside Map               -- NEXT --
//   Progression system       -- PLANNED --
//   Scnces system            -- PLANNED --
//   Sounds system            -- PLANNED --

static void border_collision(vec2s *a, vec2s size, vec4s position) {
    if (a->y < position.y) a->y = position.y;
    if (a->x < position.x) a->x = position.x;

    if (a->x + size.x > position.z) a->x = position.z - size.x;
    if (a->y + size.y > position.w) a->y = position.w - size.y;
}

static void input_handling(void) {
    player_input();

#ifdef DEBUG_INPUT
    static f32 delay = 0.0f;

    delay += global.dt;

    if (delay >= 0.15) {
        if (window_get_key(global.window, GLFW_KEY_X)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            delay = 0.0f;
        }
        else if (window_get_key(global.window, GLFW_KEY_Z)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            delay = 0.0f;
        }

        if (window_get_key(global.window, GLFW_KEY_I)) {
            global.toggle_collision = !global.toggle_collision;
            delay = 0.0f;
        }
        if (window_get_key(global.window, GLFW_KEY_U)) {
            global.toggle_show_collider = !global.toggle_show_collider;
            delay = 0.0f;
        }
        if (window_get_key(global.window, GLFW_KEY_Y)) {
            global.toggle_editor = !global.toggle_editor;
            delay = 0.0f;
        }

        if (glfwGetMouseButton(global.window->handle, GLFW_MOUSE_BUTTON_LEFT)) {
            if (global.cursor_mode == START_POINT) {
                global.start_point[0] = global.window->mouse.orthox;
                global.start_point[1] = global.window->mouse.orthoy;
            }
            else if (global.cursor_mode == END_POINT) {
                global.end_point[0] = global.window->mouse.orthox;
                global.end_point[1] = global.window->mouse.orthoy;
            }
            delay = 0.0f;
        }
        if (glfwGetMouseButton(global.window->handle, GLFW_MOUSE_BUTTON_RIGHT)) {
            global.cursor_mode = (global.cursor_mode + 1) % 3;
            delay = 0.0f;
        }
    }
#endif
}

void setup(void) {
    global.FadeState.alpha = 0.0f;
    global.FadeState.state = FADE_NONE;

    asset_manager_init(&global.asset_manager);
    renderer_init();

    /* struct DialogText text1 = {.text = "HI, WELCOME TO CVILLAGE! ARE YOU READY FOR THE QUESTION?"}; */
    /* struct DialogText text2 = {.text = "IF YOU NOT, GET OUT"}; */

    /* struct DialogQuestion question1 = { */
    /*     .answer   = {"C", "A", "F", "G"}, */
    /*     .question = "WHAT IS THE LANGUAGE WE GONNA LEARN TODAY?", */
    /*     .wrong_answer_text   = "UNFORTUNATELY, YOU'RE WRONG! TRY AGAIN.", */
    /*     .correct_answer_text = "OH! YOU'RE RIGHT! KEEP IT UP", */
    /*     .correct_answer_idx  = 0 */ 
    /* }; */

    /* dialog = dialog_create("BENNY"); */
    /* dialog_append(dialog, DIALOG_TEXT, &text1); */
    /* dialog_append(dialog, DIALOG_TEXT, &text2); */
    /* dialog_append(dialog, DIALOG_QUESTION, &question1); */
#ifdef TEST_DIALOG
    struct DialogText text1 = {.text = "HI, WELCOME TO CVILLAGE! ARE YOU READY FOR THE QUESTION?"};
    struct DialogText text2 = {.text = "IF YOU NOT, GET OUT"};

    struct DialogQuestion question1 = {
        .answer   = {"C", "A", "F", "G"},
        .question = "WHAT IS THE LANGUAGE WE GONNA LEARN TODAY?",
        .wrong_answer_text   = "UNFORTUNATELY, YOU'RE WRONG! TRY AGAIN.",
        .correct_answer_text = "OH! YOU'RE RIGHT! KEEP IT UP",
        .correct_answer_idx  = 0 
    };

    dialog = dialog_create("BENNY");
    dialog_append(dialog, DIALOG_TEXT, &text1);
    dialog_append(dialog, DIALOG_TEXT, &text2);
    dialog_append(dialog, DIALOG_QUESTION, &question1);

#endif
}

void update(void) {
    input_handling();

    /* printf("FPS: %f\tCurrentChunkId: %u\n", 1 / global.dt, global.ChunkState.chunk_id); */
    Body *player_body = physics_body_get(global.PlayerState.body_id);

    /* { // manual dialog */
    /*     static b8 set = false; */
    /*     ivec2s coord = {player_body->position.x / TILE_SIZE, player_body->position.y / TILE_SIZE }; */
    /*     /1* printf("coord: (%d,%d)\n", coord.x, coord.y); *1/ */
    /*     if (coord.x == 20 && coord.y == 20 && !set) { */
    /*         // set dialog */
    /*         global.DialogState.name             = dialog->name; */
    /*         global.DialogState.curr_dialog_node = dialog->contents; */
    /*         global.DialogState.selected_answer  = 0; */

    /*         player_body->velocity = (vec2s){0,0}; */
    /*         set = true; */
    /*     } */        
    /* } */
    { // Camera
        camera_center_to_obj(global.camera, player_body->position, PLAYER_HITBOX);

        border_collision(&global.camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, global.ChunkState.chunk->position);
        border_collision(&player_body->position, PLAYER_HITBOX, global.ChunkState.chunk->position);

        camera_update(global.camera); 
    }

    {
        mat4s inv_vp      = glms_mat4_mul(global.camera->inverse_view_proj.view, global.camera->inverse_view_proj.proj);
        vec4s ortho_mouse = glms_mat4_mulv(inv_vp, (vec4s){global.window->mouse.normalx, global.window->mouse.normaly, 0.0f, 1.0f});
        global.window->mouse.orthox = ortho_mouse.x;
        global.window->mouse.orthoy = ortho_mouse.y;
    }


    physics_update(global.dt);
    animation_update(global.dt);

    renderer_prepare();
    renderer_render();
}

void cleanup(void) {
    asset_manager_destroy(global.asset_manager);
#ifdef TEST_DIALOG
    dialog_delete(dialog);
#endif
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
