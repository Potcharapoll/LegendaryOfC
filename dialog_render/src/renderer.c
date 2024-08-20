#include "renderer.h"
#include "freetype/ftmodapi.h"
#include "renderer_internal.h"
#include "global.h"

static const int texture_slot[8] = {0,1,2,3,4,5,6,7};
static struct RenderState state;

// Initialize Freetype font and load font to the global.fonts.atlas
static void _renderer_init_font(void) {
    assert(FT_Init_FreeType(&state.font.handle) == 0);
    assert(FT_New_Face(state.font.handle, FONT5_PATH, 0, &state.font.face) == 0);
    assert(FT_Set_Pixel_Sizes(state.font.face, 0, FONT_HEIGHT) == 0);

    // Problem 1: lower case not align correctly when fixed rows cols size

    // we don't need first 32 characters in the ASCII 
    u32 count = ASCII_COUNT - 32;

    state.font.characters      = calloc(count, sizeof(*state.font.characters));
    state.font.character_count = count; 

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // NEEDED

    GL_TRY(glGenTextures(1, &global.fonts.atlas));
    GL_TRY(glBindTexture(GL_TEXTURE_2D, global.fonts.atlas));

    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_TRY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_TRY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, global.fonts.atlas_width, global.fonts.atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));

    for (u32 c = 32, idx = 0, _x = 0, _y = 0; c < ASCII_COUNT; c++, idx++) {
        assert(FT_Load_Char(state.font.face, c, FT_LOAD_RENDER) == 0);

        u32 width  = state.font.face->glyph->bitmap.width;
        u32 height = state.font.face->glyph->bitmap.rows;

        u32 cellX  = (_x % 10) * 14;
        u32 cellY  = _y * 14;

        u8 *fliped_buffer = malloc(width * height);
        for (u32 y = 0; y < height; y++)  {
            memcpy(fliped_buffer + y * width, 
                    state.font.face->glyph->bitmap.buffer + (height - 1 - y) * width, width);
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

        state.font.characters[idx].bearing = (ivec2s){state.font.face->glyph->bitmap_left, state.font.face->glyph->bitmap_top};
        state.font.characters[idx].size    = (ivec2s){14,14};
        state.font.characters[idx].uvs     = (vec4s){
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

static void _batch_render_init(struct BatchRender **batch) {
    struct BatchRender *new_batch = malloc(sizeof(*new_batch));

    new_batch->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(*new_batch->vertices));
    new_batch->quad_count    = 0;
    new_batch->texture_count = 0;

    memset(new_batch->vertices, 0, MAX_VERTICES_PER_BATCH * sizeof(*new_batch->vertices));
    memset(new_batch->textures, 0, sizeof(new_batch->textures));

    GL_TRY(glGenVertexArrays(1, &new_batch->vao));
    GL_TRY(glGenBuffers(1, &new_batch->vbo));
    GL_TRY(glGenBuffers(1, &new_batch->ebo));

    GL_TRY(glBindVertexArray(new_batch->vao));

    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, new_batch->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), NULL, GL_DYNAMIC_DRAW));

    u32 *indices = malloc(MAX_INDICES_PER_BATCH * sizeof(*indices));
    for (int i = 0; i < MAX_QUAD_PER_BATCH; i++) {
        u32 offset = i * 4;
        u32 idx    = i * 6;

        indices[idx + 0] = offset + 0;
        indices[idx + 1] = offset + 1;
        indices[idx + 2] = offset + 2;

        indices[idx + 3] = offset + 2;
        indices[idx + 4] = offset + 3;
        indices[idx + 5] = offset + 0;
    }

    GL_TRY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_batch->ebo));
    GL_TRY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES_PER_BATCH * sizeof(*indices), indices, GL_STATIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, position)));
    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, color)));
    GL_TRY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, tex_coord)));
    GL_TRY(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, tex_slot)));

    GL_TRY(glEnableVertexAttribArray(0));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glEnableVertexAttribArray(2));
    GL_TRY(glEnableVertexAttribArray(3));

    GL_TRY(glBindVertexArray(0));

    GL_TRY(new_batch->shader = glCreateProgram());
    GL_TRY(GLuint vs = _renderer_compile_shader(GL_VERTEX_SHADER, "shaders/default.vert"));
    GL_TRY(GLuint fs = _renderer_compile_shader(GL_FRAGMENT_SHADER, "shaders/default.frag"));

    GL_TRY(glAttachShader(new_batch->shader, fs));
    GL_TRY(glAttachShader(new_batch->shader, vs));

    {
        GLint rel;
        char msg[512];

        GL_TRY(glLinkProgram(new_batch->shader));
        GL_TRY(glGetProgramiv(new_batch->shader, GL_LINK_STATUS, &rel));
        if (!rel) {
            glGetProgramInfoLog(new_batch->shader, 512, NULL, msg);
            fprintf(stderr, "Program:Error:%s\n", msg);
            abort();
        }

        GL_TRY(glValidateProgram(new_batch->shader));
        GL_TRY(glGetProgramiv(new_batch->shader, GL_VALIDATE_STATUS, &rel));
        assert(rel != GL_FALSE);
    }

    GL_TRY(glDeleteShader(vs));
    GL_TRY(glDeleteShader(fs));

    free(indices);
    *batch = new_batch;
}

static void _batch_render_destroy(struct BatchRender *batch) {
    glDeleteVertexArrays(1, &batch->vao);
    glDeleteBuffers(1, &batch->vbo);
    glDeleteBuffers(1, &batch->ebo);
    glDeleteProgram(batch->shader);

    for (u32 i = 0; i < batch->texture_count; i++) {
        glDeleteTextures(1, &batch->textures[i]);
    }

    free(batch->vertices);
    free(batch);
}

// add texture to batch render and return the texture slot
static u32 _batch_render_add_texture(struct BatchRender *batch, GLuint texture) {
    b8 found     = false;
    u32 tex_slot = -1;

    for (u32 i = 0; i < state._render_batch->texture_count; i++) {
        if (batch->textures[i] == texture) {
            tex_slot = i;
            found = true;
            break;
        }
    }

    if (!found) {
        tex_slot = batch->texture_count++;
        batch->textures[tex_slot] = texture;
    } 

    return tex_slot;
}

// return texture slot of texture in batch render (Asuume that it was already added to the batch render overwise will return -1)
static u32 _batch_render_get_texture_slot(struct BatchRender *batch, GLuint texture) {
    for (u32 i = 0; i < state._render_batch->texture_count; i++) {
        if (batch->textures[i] == texture) {
            return i;
        }
    }
    return -1;
}

void renderer_init(void) {
    state.proj = glms_ortho(0.0f, WIDTH, 0.0f, HEIGHT, 0.0f, 100.0f);
    _batch_render_init(&state._render_batch);
    _renderer_init_dialog();
    _renderer_init_font();

    { // dialog batch setup
        _batch_render_init(&state._dialog_batch);
        _batch_render_add_texture(state._dialog_batch, global.fonts.atlas);
        _batch_render_add_texture(state._dialog_batch, 0);
   }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderer_destroy(void){ 
    _batch_render_destroy(state._render_batch);
    _batch_render_destroy(state._dialog_batch);

    FT_Done_Face(state.font.face);
    FT_Done_Library(state.font.handle);

    glDeleteTextures(1, &global.fonts.atlas);
    free(state.font.characters);
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    state._render_batch->quad_count = 0;
    state._dialog_batch->quad_count = 0;
}

void renderer_render(void) {

    { // render batch
        GL_TRY(glUseProgram(state._render_batch->shader));
        GL_TRY(glUniformMatrix4fv(glGetUniformLocation(state._render_batch->shader, "proj"), 1, GL_FALSE, (const f32*)state.proj.raw));
        GL_TRY(glUniform1iv(glGetUniformLocation(state._render_batch->shader, "tex"), 8, texture_slot));

        GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, state._render_batch->vbo));
        GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), state._render_batch->vertices));

        for (u32 i = 0; i < state._render_batch->texture_count; i++) {
            GL_TRY(glActiveTexture(GL_TEXTURE0 + i));
            GL_TRY(glBindTexture(GL_TEXTURE_2D, state._render_batch->textures[i]));
        }

        GL_TRY(glBindVertexArray(state._render_batch->vao));
        GL_TRY(glDrawElements(GL_TRIANGLES, (6 * state._render_batch->quad_count), GL_UNSIGNED_INT, 0));

        GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
        GL_TRY(glBindVertexArray(0));
        GL_TRY(glUseProgram(0));
    }

    { // dialog batch
        GL_TRY(glUseProgram(state._dialog_batch->shader));
        GL_TRY(glUniformMatrix4fv(glGetUniformLocation(state._dialog_batch->shader, "proj"), 1, GL_FALSE, (const f32*)state.proj.raw));
        GL_TRY(glUniform1iv(glGetUniformLocation(state._dialog_batch->shader, "tex"), 8, texture_slot));

        GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, state._dialog_batch->vbo));
        GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), state._dialog_batch->vertices));

        for (u32 i = 0; i < state._dialog_batch->texture_count; i++) {
            GL_TRY(glActiveTexture(GL_TEXTURE0 + i));
            GL_TRY(glBindTexture(GL_TEXTURE_2D, state._dialog_batch->textures[i]));
        }

        GL_TRY(glBindVertexArray(state._dialog_batch->vao));
        GL_TRY(glDrawElements(GL_TRIANGLES, (6 * state._dialog_batch->quad_count), GL_UNSIGNED_INT, 0));

        GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
        GL_TRY(glBindVertexArray(0));
        GL_TRY(glUseProgram(0));
    }
}

// append quad to _render_batch
void renderer_render_quad(vec3s position, vec2s size, vec4s color) {
    u32 idx = state._render_batch->quad_count++;

    state._render_batch->vertices[idx * 4 + 0] = (struct Vertex){ 
        .position = { .x = position.x, .y = position.y, .z = position.z}, 
        .color    = color, 
        .tex_slot = -1 
    };
    state._render_batch->vertices[idx * 4 + 1] = (struct Vertex){ 
        .position = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color    = color, 
        .tex_slot = -1 
    };
    state._render_batch->vertices[idx * 4 + 2] = (struct Vertex){ 
        .position = { .x = position.x + size.x, .y = position.y + size.y , .z = position .z}, 
        .color    = color, 
        .tex_slot = -1 
    };
    state._render_batch->vertices[idx * 4 + 3] = (struct Vertex){ 
        .position = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color    = color, 
        .tex_slot = -1 
    };
}

// append quad with texture to _render_batch
void renderer_render_quad_texture(vec3s position, vec2s size, vec4s color, u32 texture) {

    if (state._render_batch->texture_count >= 8) {
        fprintf(stderr, "Texture slot fulled\n");
        return;
    }

    b8 found     = false;
    u32 tex_slot = -1;

    for (u32 i = 0; i < state._render_batch->texture_count; i++) {
        if (state._render_batch->textures[i] == texture) {
            tex_slot = i;
            found = true;
            break;
        }
    }

    if (!found) {
        tex_slot = state._render_batch->texture_count++;
        state._render_batch->textures[tex_slot] = texture;
    } 

    u32 idx = state._render_batch->quad_count++;

    state._render_batch->vertices[idx * 4 + 0] = (struct Vertex){ 
        .position = { .x = position.x, .y = position.y, .z = position.z}, 
        .color = color, 
        .tex_coord = {0,0}, 
        .tex_slot = tex_slot 
    };
    state._render_batch->vertices[idx * 4 + 1] = (struct Vertex){ 
        .position = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color = color, 
        .tex_coord = {1,0}, 
        .tex_slot = tex_slot 
    };
    state._render_batch->vertices[idx * 4 + 2] = (struct Vertex){ 
        .position = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
        .color = color, 
        .tex_coord = {1,1}, 
        .tex_slot = tex_slot 
    };
    state._render_batch->vertices[idx * 4 + 3] = (struct Vertex){ 
        .position = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color = color, 
        .tex_coord = {0,1}, 
        .tex_slot = tex_slot 
    };
}

void renderer_render_text(vec3s position, vec4s color, char *text) {
    vec3s _pos = position;
    u32  count = state.font.character_count; 

    for (u32 idx = 0; idx < strlen(text); idx++) {
        char c = text[idx];

        if (c == '\\') {
            _pos.y -= FONT_HEIGHT + 8;
            _pos.x = position.x;
            continue;
        }

        struct Character char_info = state.font.characters[count - (ASCII_COUNT - (u8)c)];
        ivec2s _size = char_info.size;

        f32 xpos = _pos.x;
        f32 ypos = 0.0f;

        if (c == 'y')
            ypos = _pos.y - (char_info.size.y - char_info.bearing.y);
        else 
            ypos = _pos.y;

        u32 idx      = state._dialog_batch->quad_count++;
        u32 tex_slot = _batch_render_get_texture_slot(state._dialog_batch, global.fonts.atlas); 

        state._dialog_batch->vertices[idx * 4 + 0] = (struct Vertex){ 
            .position  = { .x = xpos, .y = ypos, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.x, char_info.uvs.y}, 
            .tex_slot  = tex_slot, 
        };
        state._dialog_batch->vertices[idx * 4 + 1] = (struct Vertex){ 
            .position  = { .x = xpos + _size.x, .y = ypos, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.z, char_info.uvs.y}, 
            .tex_slot  = tex_slot,
        };
        state._dialog_batch->vertices[idx * 4 + 2] = (struct Vertex){ 
            .position  = { .x = xpos + _size.x, .y = ypos + _size.y, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.z, char_info.uvs.w}, 
            .tex_slot  = tex_slot,
        };
        state._dialog_batch->vertices[idx * 4 + 3] = (struct Vertex){ 
            .position  = { .x = xpos, .y = ypos + _size.y, .z = _pos.z}, 
            .color     = color, 
            .tex_coord = {char_info.uvs.x, char_info.uvs.w}, 
            .tex_slot  = tex_slot,
        };

        _pos.x += _size.x;
    }
}

/* void renderer_render_dialog_text_animation(vec3s position, vec4s color, char *text) { */
/*     vec3s _pos = position; */
/*     u32  count = state.font.character_count; */ 

/*     GL_TRY(glActiveTexture(GL_TEXTURE0)); */
/*     GL_TRY(glBindTexture(GL_TEXTURE_2D, global.fonts.atlas)); */

/*     GL_TRY(glBindVertexArray(state.vao)); */

/*     for (u32 idx = 0; idx < global.DialogState.curr_animation_idx; idx++) { */
/*         char c = text[idx]; */

/*         if (c == '\\') { */
/*             _pos.y -= (FONT_HEIGHT + 16); */
/*             _pos.x = position.x; */
/*             continue; */
/*         } */

/*         struct Character char_info = state.font.characters[count - (ASCII_COUNT - (u8)c)]; */
/*         ivec2s _size = char_info.size; */

/*         GL_TRY(glUseProgram(state.shader)); */
/*         GL_TRY(glUniformMatrix4fv(glGetUniformLocation(state.shader, "proj"), 1, GL_FALSE, (const f32*)state.proj.raw)); */
/*         GL_TRY(glUniform1i(glGetUniformLocation(state.shader, "tex"), 0)); */

/*         f32 xpos = _pos.x; */
/*         f32 ypos = 0.0f; */

/*         if (c == 'y' || c == 'g' || c == 'p' || c == 'q') */
/*             ypos = _pos.y - (char_info.size.y - char_info.bearing.y); */
/*         else */ 
/*             ypos = _pos.y; */

/*         struct Vertex vertices[4] = { */
/*             { .position = { .x = xpos,           .y = ypos,           .z = _pos.z}, .color = color, .tex_coord = {char_info.uvs.x, char_info.uvs.y}, .has_texture = 1 }, */
/*             { .position = { .x = xpos + _size.x, .y = ypos,           .z = _pos.z}, .color = color, .tex_coord = {char_info.uvs.z, char_info.uvs.y}, .has_texture = 1 }, */
/*             { .position = { .x = xpos + _size.x, .y = ypos + _size.y, .z = _pos.z}, .color = color, .tex_coord = {char_info.uvs.z, char_info.uvs.w}, .has_texture = 1 }, */
/*             { .position = { .x = xpos,           .y = ypos + _size.y, .z = _pos.z}, .color = color, .tex_coord = {char_info.uvs.x, char_info.uvs.w}, .has_texture = 1 }, */
/*         }; */

/*         GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, state.vbo)); */
/*         GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)); */

/*         GL_TRY(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)); */

/*         _pos.x += _size.x; */
/*     } */

/*     GL_TRY(glBindTexture(GL_TEXTURE_2D, 0)); */
/*     GL_TRY(glBindVertexArray(0)); */
/*     GL_TRY(glUseProgram(0)); */

/*     if (global.DialogState.curr_animation_idx < strlen(text)) { */
/*         global.DialogState.onAnimation         = true; */
/*         global.DialogState.curr_animation_idx += 1; */
/*     } */
/*     else { */
/*         global.DialogState.onAnimation = false; */
/*     } */
/* } */

// add triangle to _render_batch
// STATUS: Broken - cannot put it together with _render_batch because render batch use for render quad.
// TODO: Create batch for triangle.
void renderer_render_triangle(vec3s position, vec2s size, vec4s color) {
    // SUGGEST: should I make a function to feed a custom vertices

    u32 idx = state._render_batch->quad_count++;
    state._render_batch->vertices[idx * 4 + 0] = (struct Vertex){ .position = { .x = position.x,                .y = position.y,          .z = position.z}, .color = color, .tex_slot = -1 };
    state._render_batch->vertices[idx * 4 + 1] = (struct Vertex){ .position = { .x = position.x + size.x,       .y = position.y,          .z = position.z}, .color = color, .tex_slot = -1 };
    state._render_batch->vertices[idx * 4 + 2] = (struct Vertex){ .position = { .x = position.x + size.x * 0.5, .y = position.y - size.y, .z = position.z}, .color = color, .tex_slot = -1 };
}
