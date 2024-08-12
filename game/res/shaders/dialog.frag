#version 330 core
out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D text;

void main() {
    float alpha = texture(text, tex_coord).r;
    frag_color  = vec4(1.0, 1.0, 1.0, 1.0);
}
