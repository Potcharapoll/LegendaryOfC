#ifndef DIALOG_H
#define DIALOG_H
#define DIALOG_FRAME_COLOR (vec4s){0,0,0,0.3}
#define DIALOG_TEXT_COLOR  (vec4s){1,1,1,1}
#define DIALOG_FRAME_SIZE  (vec2s){PROJECTION_WIDTH, 75.0f}
#define DIALOG_SELECT_SIZE (vec2s){2,2}
#define DIALOG_TEXT_SIZE   (vec2s){8,8}

#include "../util/types.h"

enum DialogType {
    DIALOG_TEXT,
    DIALOG_QUESTION,

    DIALOG_LAST
};

struct DialogQuestion {
    char *question;

    char *answer[4];
    u32 correct_answer_idx;

    char *correct_answer_text;
    char *wrong_answer_text;
};

struct DialogText { char *text; };

struct DialogNode {
    enum DialogType type;
    void *dialog;

    struct DialogNode *next;
};

struct Dialog {
    char *name;
    struct DialogNode *contents;
};

void dialog_init(void);
void dialog_destroy(void);
struct Dialog* dialog_create(char *name);
void dialog_delete(struct Dialog *dialog);
void dialog_append(struct Dialog *dialog, enum DialogType type, void *data);
void dialog_render(void);
void dialog_input(void);
#endif
