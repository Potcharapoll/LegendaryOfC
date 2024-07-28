#version 330 core
out vec4 fragColor;

in vec4 color;
in vec2 tex_coord;
in float tex_slot;
uniform sampler2D tex;

void main() {
    if (tex_slot == 0) {
        fragColor = color;
    }
    else {
        fragColor = texture(tex, tex_coord) * color;
    }
}
