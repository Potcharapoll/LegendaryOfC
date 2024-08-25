#version 330 core
out vec4 fragColor;

in vec4 color;
in vec2 tex_coord;

flat in int tex_slot;

uniform sampler2D tex[8];

void main() {
    switch (tex_slot) {
        case -1:
            fragColor = color;
            break;
        case 0:
            fragColor = texture(tex[0], tex_coord);
            break;
        case 1:
            fragColor = texture(tex[1], tex_coord);
            break;
        case 2:
            fragColor = texture(tex[2], tex_coord);
            break;
        case 3:
            fragColor = texture(tex[3], tex_coord);
            break;
        case 4:
            fragColor = texture(tex[4], tex_coord);
            break;
        case 5:
            fragColor = texture(tex[5], tex_coord);
            break;
        case 6:
            fragColor = texture(tex[6], tex_coord);
            break;
        case 7:
            fragColor = texture(tex[7], tex_coord);
            break;
    }
}
