#ifndef DIALOG_H
#define DIALOG_H
#define DIALOG_FRAME_COLOR (vec4s){0,0,0,0.3}
#define DIALOG_TEXT_COLOR  (vec4s){1,1,1,1}
#define DIALOG_FRAME_SIZE  (vec2s){PROJECTION_WIDTH, 75.0f}
#define DIALOG_SELECT_SIZE (vec2s){2,2}
#define DIALOG_TEXT_SIZE   (vec2s){8,8}
#include "../util/types.h"

enum DialogType {
    DIALOG_TYPE_TEXT,
    DIALOG_TYPE_QUESTION,

    DIALOG_TYPE_LAST
};

typedef struct {
    char *question;
    char *answer[4];
    u8 correct_idx;

    char *correct_text;
    char *wrong_text;
}DialogQuestion;

typedef struct { 
    char *text; 
} DialogText;

typedef struct DialogNode {
    char *name;
    enum DialogType type;
    void *dialog;

    struct DialogNode *next;
}DialogNode;

typedef struct Dialog {
    u32 length;
    DialogNode *contents;
} Dialog;

Dialog* dialog_create(void);
Dialog* dialog_load_from_file(char *path);
void dialog_delete(Dialog *dialog);

void dialog_append(Dialog *dialog, char *name, enum DialogType type, void *data);
void dialog_render(void);
void dialog_input(void);
#endif
