#version 330

uniform bool wireframeFlag;
uniform sampler2D grassTexturer;

in float brightness;
in vec2 texCoord;

void main() {
    vec4 texColor = texture(grassTexturer, texCoord);
    gl_FragColor = wireframeFlag ? vec4(0, 0, 0, 1) : texColor * brightness;
}
