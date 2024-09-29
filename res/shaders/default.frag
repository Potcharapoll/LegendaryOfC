#version 330 core
out vec4 frag_color;

in vec4 color;
in vec2 tex_coord;

flat in int tex_slot;

uniform sampler2D tex[8];

void main() {
    switch (tex_slot) {
        case -1:
            frag_color = color;
            break;
        case 0:
            frag_color = color * texture(tex[0], tex_coord);
            break;
        case 1:
            frag_color = color * texture(tex[1], tex_coord);
            break;
        case 2:
            frag_color = color * texture(tex[2], tex_coord);
            break;
        case 3:
            frag_color = color * texture(tex[3], tex_coord);
            break;
        case 4:
            frag_color = color * texture(tex[4], tex_coord);
            break;
        case 5:
            frag_color = color * texture(tex[5], tex_coord);
            break;
        case 6:
            frag_color = color * texture(tex[6], tex_coord);
            break;
        case 7:
            frag_color = color * texture(tex[7], tex_coord);
            break;
    }
}
