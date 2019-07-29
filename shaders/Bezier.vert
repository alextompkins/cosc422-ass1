#version 330

layout (location = 0) in vec4 position;

uniform mat4 mvpMatrix;

void main() {
    gl_Position = position;
}