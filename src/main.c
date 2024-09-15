#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/renderer.h"
#include "core/dialog.h"

/* #define DEBUG_INPUT */
/* #define TEST_DIALOG */

#include "global.h"
#include "defs.h"

//   ~70%
//   Teleport&Dialog Callback -- ON GOING --
//   Map                      -- PLANNED  --
//   Progression system       -- PLANNED  --
//   Scnces system            -- PLANNED  --
//   Sounds system            -- PLANNED  --
//   Fade in/fade out (**IF POSSIBLE)

//   Attach body to prefab

//   Main Program #1 Batch Render & Layering

struct Dialog *dialog;

static void border_collision(vec2s *a, vec2s size, vec4s position) {
    if (a->y < position.y) a->y = position.y;
    if (a->x < position.x) a->x = position.x;

    if (a->x + size.x > position.z) a->x = position.z - size.x;
    if (a->y + size.y > position.w) a->y = position.w - size.y;
}

static void input_handling(void) {
    if (global.DialogState.curr_dialog_node) {
        dialog_input();
    }
    else {
        player_input();
    }

#ifdef DEBUG_INPUT
    if (window_get_key(global.window, GLFW_KEY_X)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (window_get_key(global.window, GLFW_KEY_Z)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif
}

void setup(void) {
    asset_manager_init(&global.asset_manager);
    renderer_init();

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

    // set dialog
    global.DialogState.name             = dialog->name;
    global.DialogState.curr_dialog_node = dialog->DialogList.dialog;
    global.DialogState.selected_answer  = 0;
#endif
}

void update(void) {
    input_handling();

    printf("FPS: %f\n", 1 / global.dt);

    { // Camera
        Body *player_body = physics_body_get(global.PlayerState.body_id);
        camera_center_to_obj(global.camera, player_body->position, PLAYER_HITBOX);

        border_collision(&global.camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, global.ChunkState.chunk->position);
        border_collision(&player_body->position, PLAYER_HITBOX, global.ChunkState.chunk->position);

        camera_update(global.camera); 
    }

    physics_update(global.dt);
    animation_update(global.dt);

    renderer_prepare();
    renderer_render();
}

void cleanup(void) {

#ifdef TEST_DIALOG
    dialog_delete(dialog);
#endif

    asset_manager_destroy(global.asset_manager);
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
