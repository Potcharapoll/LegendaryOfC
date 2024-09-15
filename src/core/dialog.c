#include "../global.h"
#include "../defs.h"

#include "dialog.h"
#include "asset_manager.h"
#include "batch_render.h"

#include <string.h>

typedef enum {
    DIALOG_POSITION_TITLE,
    DIALOG_POSITION_TEXT,
    DIALOG_POSITION_ANSWER1,
    DIALOG_POSITION_ANSWER2,
    DIALOG_POSITION_ANSWER3,
    DIALOG_POSITION_ANSWER4,

    DIALOG_POSITION_LAST,
} DialogRenderPosition;

static vec3s dialog_render_position[DIALOG_POSITION_LAST] = {
    { 24,       120,     0.0f},
    { 25,       90,      0.0f},
    { 25 + 50,  90 - 28, 0.0f},
    { 25 + 50,  90 - 58, 0.0f},
    { 25 + 300, 90 - 28, 0.0f},
    { 25 + 300, 90 - 58, 0.0f},
};

static b8                  next_dialog   = true;
static b8                  animation_end = false;
static GLuint              _vbo          = GL_NONE;
static struct BatchRender *_dialog_batch = NULL;

static ivec2s get_char_coord(char c) {
    static u8 text_index[3][27] = {
        "!@#$%^&*()_+-={}[]:\";\'<>,.?",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ/",
        "abcdefghijklmnopqrstuvwxyz ",
    };

    ivec2s result = {0};

    for (u8 y = 0; y < 3; ++y) {
        for (u8 x = 0; x < 27; ++x) {
            if (c == text_index[y][x]) {
                result.x = x;
                result.y = y;
                break;
            }
        }
    }


    return result;
}

static void dialog_render_text(char *text, vec2s size, vec3s pos, vec4s color) {
    f32 tex_coord[4];

    struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT);
    vec2s cell_size = {(f32)sp->stride / sp->texture.size.x, (f32)sp->stride / sp->texture.size.y};

    for (u32 idx = 0; idx < strlen(text); ++idx) {
        ivec2s char_coord = get_char_coord(text[idx]);

        tex_coord[0] = cell_size.x * char_coord.x;
        tex_coord[1] = cell_size.x * char_coord.x + cell_size.x;
        tex_coord[2] = cell_size.y * char_coord.y;
        tex_coord[3] = cell_size.y * char_coord.y + cell_size.y;

        batch_render_append_quad_texture(_dialog_batch, pos, size, color, sp->texture, tex_coord);
        pos.x += (size.x * 0.5);
    }
}

static void dialog_render_text_animation(char *text, vec2s size, vec3s pos, vec4s color) {
    static u32 current_rendered_idx = 0;
    static u32 max_rendered_idx     = 0;


    if (next_dialog) {
        current_rendered_idx = 0;
        max_rendered_idx     = strlen(text);
        next_dialog          = false;
        animation_end        = false;
    }
    

    f32 tex_coord[4];

    struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT);
    vec2s cell_size = {(f32)sp->stride / sp->texture.size.x, (f32)sp->stride / sp->texture.size.y};

    for (u32 idx = 0; idx < current_rendered_idx; ++idx) {
        ivec2s char_coord = get_char_coord(text[idx]);

        tex_coord[0] = cell_size.x * char_coord.x;
        tex_coord[1] = cell_size.x * char_coord.x + cell_size.x;
        tex_coord[2] = cell_size.y * char_coord.y;
        tex_coord[3] = cell_size.y * char_coord.y + cell_size.y;

        batch_render_append_quad_texture(_dialog_batch, pos, size, color, sp->texture, tex_coord);
        pos.x += (size.x * 0.5);
    }

    if (current_rendered_idx < max_rendered_idx) {
        current_rendered_idx++;
    }
    else {
        animation_end = true;
    }
}

void dialog_init(void) {
    { // specific buffer for triangle
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex[4]), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    _dialog_batch = batch_render_init();

    struct Spritesheet *sp = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TEXT);
    batch_render_append_texture(_dialog_batch, sp->texture);
}

void dialog_destroy(void) {
    batch_render_destroy(_dialog_batch);
}

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

// render when global.DialogState.dialog != NULL
void dialog_render(void) {
    vec3s camera_pos = { global.camera->position.x, global.camera->position.y, 0.0f};
    vec3s pos        = {0};

    batch_render_append_quad(_dialog_batch, camera_pos, DIALOG_FRAME_SIZE, DIALOG_FRAME_COLOR);

    pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TITLE]);
    dialog_render_text(global.DialogState.name, (vec2s){20,21}, pos, BLUE);
    dialog_render_text(global.DialogState.name, (vec2s){20,20}, pos, DIALOG_TEXT_COLOR);

    pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TEXT]);

    if (global.DialogState.curr_dialog_node->type == DIALOG_QUESTION) {
        struct DialogQuestion *content = global.DialogState.curr_dialog_node->dialog;
        dialog_render_text_animation(content->question, (vec2s){12,12}, pos, DIALOG_TEXT_COLOR);

        u8 selected_answer = DIALOG_POSITION_LAST - 4 + global.DialogState.selected_answer;
        if (animation_end) {
            pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER1]);
            dialog_render_text(content->answer[0], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR);

            pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER2]);
            dialog_render_text(content->answer[1], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR);

            pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER3]);
            dialog_render_text(content->answer[2], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR);

            pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER4]);
            dialog_render_text(content->answer[3], DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR);

            pos = glms_vec3_add(camera_pos, glms_vec3_sub(dialog_render_position[selected_answer], (vec3s){20, -6, 0}));
            batch_render_append_quad(_dialog_batch, pos, (vec2s){6,6}, DIALOG_TEXT_COLOR);
        }
    }
    else {
        struct DialogText *content = global.DialogState.curr_dialog_node->dialog;
        dialog_render_text_animation(content->text, (vec2s){12,12}, pos, DIALOG_TEXT_COLOR);
    }

    batch_render_render(_dialog_batch);
    _dialog_batch->quad_count = 0;
}

