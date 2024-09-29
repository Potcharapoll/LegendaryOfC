#version 330 core
out vec4 frag_color;

in vec4 color;
in vec2 tex_coord;

uniform sampler2D slot;

void main() {
    frag_color  = color * texture(slot, tex_coord);
}
