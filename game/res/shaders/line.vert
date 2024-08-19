#version 330 core
layout (location = 0) in vec3  a_pos;

uniform mat4 proj;
uniform mat4 view;

void main() {
    color       = a_color;
    gl_Position = proj * view * vec4(a_pos, 1.0);
}
