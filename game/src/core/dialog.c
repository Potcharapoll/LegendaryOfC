#include "dialog.h"
#include "../util/log.h"
#include "../gfx/texture.h"
#include "../defs.h"

#include <ft2build.h>
#include FT_FREETYPE_H

static struct Texture _dialog_tex = {0};
static FT_Library _lib;
static FT_Face    _font;
static character_info_t *_characters_info = NULL;

void dialog_init(void) {
    if (FT_Init_FreeType(&_lib)) {
        LOG_ERROR("Failed to initialize FreeType Library");
        return;
    }
    if (FT_New_Face(_lib, FONT2, 0, &_font)) {
        LOG_ERROR("Failed to load font");
        return;
    }

    FT_Set_Pixel_Sizes(_font, 0, 48);

    // Fix later
    _characters_info = calloc(127, sizeof(*_characters_info));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (u8 c = 0; c < 127; c++) {
        if (FT_Load_Char(_font, c, FT_LOAD_RENDER)) {
            LOG_ERROR("Failed to load glyph of character %c", c);
            continue;
        }

        u32 width  = _font->glyph->bitmap.width;
        u32 height = _font->glyph->bitmap.rows;

        _characters_info[c].texture_id = texture_character(width, height, _font->glyph->bitmap.buffer);
        _characters_info[c].size       = (ivec2s){width, height};
        _characters_info[c].bearing    = (ivec2s){_font->glyph->bitmap_left, _font->glyph->bitmap_top};
        _characters_info[c].advance    = _font->glyph->advance.x;
    }

    _dialog_tex = texture_load("res/tilesets/dialog.png"); 
}

void dialog_destroy(void) {
    for (int i = 0; i < 127; i++) glDeleteTextures(1, &_characters_info[i].texture_id);

    texture_destroy(_dialog_tex);
    free(_characters_info);
    FT_Done_Face(_font);
    FT_Done_FreeType(_lib);
}

dialog_t* dialog_create(f32 delay, u32 question_id, dialog_desc_t desc) {
    dialog_t *dialog         = malloc(sizeof(*dialog));
    dialog->delay            = delay;
    dialog->question_id      = question_id;
    dialog->texts            = malloc(desc.count * sizeof(*dialog->texts));
    dialog->current_text_idx = 0;
    dialog->position         = (vec2s){200, 100};

    for (u32 i = 0; i < desc.count; i++) {
        dialog->texts[i].text   = desc.texts[i].text;
        dialog->texts[i].length = desc.texts[i].length;
    }

    return dialog;
}

void dialog_delete(dialog_t *dialog) {
    free(dialog->texts);
    free(dialog);
}

character_info_t get_character_info(char c) {
    return _characters_info[(u8)c];
}
