#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_tex_coord;
layout (location = 3) in float a_tex_slot;

out vec4 color;
out vec2 tex_coord;
flat out int tex_slot;

uniform mat4 proj;

void main() {
    color       = a_color;
    tex_coord   = a_tex_coord;
    tex_slot    = int(a_tex_slot);
    gl_Position = proj * vec4(a_pos, 1.0);
}
