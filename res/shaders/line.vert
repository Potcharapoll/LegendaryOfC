#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

uniform mat4 proj;
uniform mat4 view;

out vec4 color;

void main() {
    color = a_color;

    gl_Position = proj * view * vec4(a_pos, 0.0, 1.0);
}
