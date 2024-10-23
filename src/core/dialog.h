#ifndef DIALOG_H
#define DIALOG_H
#define DIALOG_FRAME_COLOR (vec4s){0,0,0,0.3}
#define DIALOG_TEXT_COLOR  (vec4s){1,1,1,1}
#define DIALOG_FRAME_SIZE  (vec2s){PROJECTION_WIDTH, 75.0f}
#define DIALOG_SELECT_SIZE (vec2s){2,2}
#define DIALOG_TEXT_SIZE   (vec2s){8,8}
#include "../util/types.h"

typedef enum {
    DIALOG_TYPE_TEXT     = 0,
    DIALOG_TYPE_QUESTION = 1,
}DialogType;
#define DIALOG_TYPE_LAST (DIALOG_TYPE_QUESTION + 1)

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
    char       name[20];
    DialogType type;
    void      *dialog;

    struct DialogNode *next;
}DialogNode;

typedef struct Dialog {
    u32         length;
    DialogNode *contents;
} Dialog;

typedef struct {
  char *dialog_tag;
  b8    append_act;
} DialogPacket;

DialogPacket* dialog_packet_create(char *tag, b8 append_act);
void dialog_packet_free(DialogPacket **packet);

Dialog* dialog_load_from_file(char *path);
DialogQuestion* dialog_load_question_from_file(char *path);
void dialog_delete(Dialog *dialog);

void dialog_append_last(Dialog *dialog, char *name, DialogType type, void *data);
void dialog_append_question_from_file(Dialog *dialog, char *name, char *path);
void dialog_append(Dialog *dialog, char *name, DialogType type, void *data);
void dialog_render(void);
void dialog_input(void);

void dialog_list(Dialog *dialog);

DialogText* _get_new_dialog_text(char *txt);
DialogQuestion* _get_new_dialog_question(char *question, char *answer[4], char *wrong_txt, char *correct_txt, u8 correct_idx);
#endif
