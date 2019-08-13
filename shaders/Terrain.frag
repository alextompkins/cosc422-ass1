#version 330

uniform bool wireframeFlag;

in vec4 primColor;

void main() {
    gl_FragColor = wireframeFlag ? vec4(0, 0, 0, 1) : primColor;
}
