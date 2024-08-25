#include "renderer.h"
#include "batch_render.h"
#include "freetype/ftmodapi.h"
#include "renderer_internal.h"
#include "global.h"

struct Character {
    ivec2s bearing;
    ivec2s size;
    vec4s  uvs; // left (2), right (2)
};

static struct BatchRender *_render_batch;
static struct Character *characters; 
static u32 character_count;
static FT_Library handle;
static FT_Face    face;

// Initialize Freetype font and load font to the global.fonts.atlas
static void _renderer_init_font(void) {
    assert(FT_Init_FreeType(&handle) == 0);
    assert(FT_New_Face(handle, FONT5_PATH, 0, &face) == 0);
    assert(FT_Set_Pixel_Sizes(face, 0, FONT_HEIGHT) == 0);

    // Problem 1: lower case not align correctly when fixed rows cols size

    // we don't need first 32 characters in the ASCII 
    u32 count = ASCII_COUNT - 32;

    characters      = calloc(count, sizeof(*characters));
    character_count = count; 

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // NEEDED

    GL_TRY(glGenTextures(1, &global.fonts.atlas));
    GL_TRY(glBindTexture(GL_TEXTURE_2D, global.fonts.atlas));

    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_TRY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, global.fonts.atlas_width, global.fonts.atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));

    for (u32 c = 32, idx = 0, _x = 0, _y = 0; c < ASCII_COUNT; c++, idx++) {
        assert(FT_Load_Char(face, c, FT_LOAD_RENDER) == 0);

        u32 width  = face->glyph->bitmap.width;
        u32 height = face->glyph->bitmap.rows;

        u32 cellX  = (_x % 10) * 14;
        u32 cellY  = _y * 14;

        u8 *fliped_buffer = malloc(width * height);
        for (u32 y = 0; y < height; y++)  {
            memcpy(fliped_buffer + y * width, 
                    face->glyph->bitmap.buffer + (height - 1 - y) * width, width);
        }

        if (c == '\'' ||  c == '\"') {
            f32 glyphOffsetX = (14 - width) / 2.0f;
            f32 glyphOffsetY = 14 - height;

            GL_TRY(glTexSubImage2D(GL_TEXTURE_2D, 0, cellX + glyphOffsetX, cellY + glyphOffsetY, width, height, GL_RED, GL_UNSIGNED_BYTE, fliped_buffer));
        }
        else {
            GL_TRY(glTexSubImage2D(GL_TEXTURE_2D, 0, cellX, cellY, width, height, GL_RED, GL_UNSIGNED_BYTE, fliped_buffer));
        }

        free(fliped_buffer);

        characters[idx].bearing = (ivec2s){face->glyph->bitmap_left, face->glyph->bitmap_top};
        characters[idx].size    = (ivec2s){14,14};
        characters[idx].uvs     = (vec4s){
            .x = (f32)cellX  / global.fonts.atlas_width,
                .y = (f32)cellY  / global.fonts.atlas_height,
                .z = ((f32)cellX + 14)  / global.fonts.atlas_width,
                .w = ((f32)cellY + 13) / global.fonts.atlas_height
        };

        _x++;
        if (_x >= 10) {
            _x = 0;
            _y++;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer_init(void) {
    global.proj         = glms_ortho(0.0f, WIDTH, 0.0f, HEIGHT, 0.0f, 100.0f);
    _render_batch = batch_render_init();

    _renderer_init_font();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderer_destroy(void){ 
    batch_render_destroy(_render_batch);

    FT_Done_Face(face);
    FT_Done_Library(handle);

    glDeleteTextures(1, &global.fonts.atlas);
    free(characters);
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    _render_batch->quad_count = 0;
}

void renderer_render(void) {
    batch_render_render(_render_batch);
}

void renderer_render_text(vec3s position, vec4s color, char *text) {
    vec3s _pos = position;
    u32  count = character_count; 

    for (u32 idx = 0; idx < strlen(text); idx++) {
        char c = text[idx];

        if (c == '\\') {
            _pos.y -= FONT_HEIGHT + 8;
            _pos.x = position.x;
            continue;
        }

        struct Character char_info = characters[count - (ASCII_COUNT - (u8)c)];
        ivec2s _size = char_info.size;

        f32 xpos = _pos.x;
        f32 ypos = 0.0f;

        if (c == 'y')
            ypos = _pos.y - (char_info.size.y - char_info.bearing.y);
        else 
            ypos = _pos.y;

        u32 idx      = _render_batch->quad_count++;
        u32 tex_slot = batch_render_get_texture_slot(_render_batch, global.fonts.atlas); 
        if (tex_slot == (u32)-1) {
            batch_render_append_texture(_render_batch, global.fonts.atlas);
        }

        _render_batch->vertices[idx * 4 + 0] = (struct Vertex){ 
            .position  = { .x = xpos, .y = ypos, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.x, char_info.uvs.y}, 
            .tex_slot  = tex_slot, 
        };
        _render_batch->vertices[idx * 4 + 1] = (struct Vertex){ 
            .position  = { .x = xpos + _size.x, .y = ypos, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.z, char_info.uvs.y}, 
            .tex_slot  = tex_slot,
        };
        _render_batch->vertices[idx * 4 + 2] = (struct Vertex){ 
            .position  = { .x = xpos + _size.x, .y = ypos + _size.y, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.z, char_info.uvs.w}, 
            .tex_slot  = tex_slot,
        };
        _render_batch->vertices[idx * 4 + 3] = (struct Vertex){ 
            .position  = { .x = xpos, .y = ypos + _size.y, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.x, char_info.uvs.w}, 
            .tex_slot  = tex_slot,
        };

        _pos.x += _size.x;
    }
}

void renderer_render_dialog_text_animation(vec3s position, vec4s color, char *text) {
    vec3s _pos = position;
    u32  count = character_count; 

    for (u32 idx = 0; idx < global.DialogState.curr_animation_idx; idx++) {
        char c = text[idx];

        if (c == '\\') {
            _pos.y -= (FONT_HEIGHT + 16);
            _pos.x = position.x;
            continue;
        }

        struct Character char_info = characters[count - (ASCII_COUNT - (u8)c)];
        ivec2s _size = char_info.size;

        f32 xpos = _pos.x;
        f32 ypos = 0.0f;

        if (c == 'y' || c == 'g' || c == 'p' || c == 'q')
            ypos = _pos.y - (char_info.size.y - char_info.bearing.y);
        else 
            ypos = _pos.y;

        u32 idx      = _render_batch->quad_count++;
        u32 tex_slot = batch_render_get_texture_slot(_render_batch, global.fonts.atlas); 

        _render_batch->vertices[idx * 4 + 0] = (struct Vertex){ 
            .position  = { .x = xpos, .y = ypos, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.x, char_info.uvs.y}, 
            .tex_slot  = tex_slot, 
        };
        _render_batch->vertices[idx * 4 + 1] = (struct Vertex){ 
            .position  = { .x = xpos + _size.x, .y = ypos, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.z, char_info.uvs.y}, 
            .tex_slot  = tex_slot,
        };
        _render_batch->vertices[idx * 4 + 2] = (struct Vertex){ 
            .position  = { .x = xpos + _size.x, .y = ypos + _size.y, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.z, char_info.uvs.w}, 
            .tex_slot  = tex_slot,
        };
        _render_batch->vertices[idx * 4 + 3] = (struct Vertex){ 
            .position  = { .x = xpos, .y = ypos + _size.y, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.x, char_info.uvs.w}, 
            .tex_slot  = tex_slot,
        };

        _pos.x += _size.x;
    }

    GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
    GL_TRY(glBindVertexArray(0));
    GL_TRY(glUseProgram(0));

    if (global.DialogState.curr_animation_idx < strlen(text)) {
        global.DialogState.onAnimation         = true;
        global.DialogState.curr_animation_idx += 1;
    }
    else {
        global.DialogState.onAnimation = false;
    }
}

void renderer_render_quad(vec3s position, vec2s size, vec4s color) {
    batch_render_append_quad(_render_batch, position, size, color);
}

void renderer_render_quad_texture(vec3s position, vec2s size, vec4s color, u32 texture) {
    batch_render_append_quad_texture(_render_batch, position, size, color, texture);
}

// add triangle to _render_batch
// STATUS: Broken - cannot put it together with _render_batch because render batch use for render quad.
// TODO: Create batch for triangle.
void renderer_render_triangle(vec3s position, vec2s size, vec4s color) {
    // SUGGEST: should I make a function to feed a custom vertices

    u32 idx = _render_batch->quad_count++;
    _render_batch->vertices[idx * 4 + 0] = (struct Vertex){ .position = { .x = position.x,                .y = position.y,          .z = position.z}, .color = color, .tex_slot = -1 };
    _render_batch->vertices[idx * 4 + 1] = (struct Vertex){ .position = { .x = position.x + size.x,       .y = position.y,          .z = position.z}, .color = color, .tex_slot = -1 };
    _render_batch->vertices[idx * 4 + 2] = (struct Vertex){ .position = { .x = position.x + size.x * 0.5, .y = position.y - size.y, .z = position.z}, .color = color, .tex_slot = -1 };
}
