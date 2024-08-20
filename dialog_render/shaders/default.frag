#version 330 core
out vec4 frag_color;

in vec4 color;
in vec2 tex_coord;
flat in int  tex_slot;

uniform sampler2D tex[8];

void main() {
    vec4 sampled;

    switch (tex_slot) {
        case -1:
            sampled    = vec4(1.0, 1.0, 1.0, 1.0);
            frag_color = sampled * color;
            break;
        case 0:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[0], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 1:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[1], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 2:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[2], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 3:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[3], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 4:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[4], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 5:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[5], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 6:
            sampled      = vec4(1.0, 1.0, 1.0, texture(tex[6], tex_coord).r);
            frag_color   = sampled * color;
            break;
        case 7:
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex[7], tex_coord).r);
            frag_color   = sampled * color;
            break;
    }
}
