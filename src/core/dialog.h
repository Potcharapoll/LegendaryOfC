#ifndef DIALOG_H
#define DIALOG_H
#include "../util/types.h"

enum DialogType {
    DIALOG_TEXT,
    DIALOG_QUESTION
};

struct DialogQuestion {
    char *question;

    char *answer[4];
    u32 corrent_answer_idx;

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
    b8 hidden_name;

    struct {
        struct DialogNode *dialog; 
        u32 count;
    } DialogList;
};

void dialog_init(void);
void dialog_destroy(void);
struct Dialog* dialog_create(char *name);
void dialog_delete(struct Dialog *dialog);
void dialog_append(struct Dialog *dialog, enum DialogType type, void *data);
void dialog_render(void);
void dialog_input(void);
#endif
