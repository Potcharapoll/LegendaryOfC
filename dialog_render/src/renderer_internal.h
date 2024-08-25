#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cglm/struct.h>
#include <glad/glad.h>

#define FONT1_PATH  "fonts/JetBrainsMonoNerdFont-Medium.ttf"
#define FONT2_PATH  "fonts/UniversCondensed.ttf"
#define FONT3_PATH  "fonts/Raleway-Bold.ttf"
#define FONT4_PATH  "fonts/Raleway-Medium.ttf"
#define FONT5_PATH  "fonts/PressStart2P-Regular.ttf"
#define FONT_HEIGHT 14
#define ASCII_COUNT 127

// Choice 1: Raleway-Bold with 28px
// Choice 2: PressStart2P-Regular with

GLuint _renderer_compile_shader(GLenum type, char *path);
#endif
