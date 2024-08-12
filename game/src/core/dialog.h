#ifndef DIALOG_H
#define DIALOG_H
#include "../util/types.h"
#include <cglm/types-struct.h>
 
typedef struct {
    char *text;
    u32 length;
} dialog_text_t;

typedef struct {
    dialog_text_t *texts;
    u32 current_text_idx;
    f32 delay;

    vec2s position;

    // if have a question; def is -1
    u32 question_id;
} dialog_t;

typedef struct {
    u32 count;
    dialog_text_t *texts;
} dialog_desc_t;

typedef struct {
    u32    texture_id;
    ivec2s size;
    ivec2s bearing;
    u32    advance;
} character_info_t;

void dialog_init(void);
dialog_t* dialog_create(f32 delay, u32 question_id, dialog_desc_t desc);
void dialog_delete(dialog_t *dialog);
void dialog_destroy(void);
character_info_t get_character_info(char c);

#endif
