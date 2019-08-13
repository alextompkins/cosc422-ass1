#version 330

uniform bool wireframeFlag;
uniform sampler2D waterTexturer;
uniform sampler2D grassTexturer;
uniform sampler2D rockTexturer;
uniform sampler2D snowTexturer;

in float brightness;
in vec4 texWeights;
in vec2 texCoord;

void main() {
    vec4 waterColor = texture(waterTexturer, texCoord);
    vec4 grassColor = texture(grassTexturer, texCoord);
    vec4 rockColor = texture(rockTexturer, texCoord);
    vec4 snowColor = texture(snowTexturer, texCoord);

    gl_FragColor = wireframeFlag ? vec4(0, 0, 0, 1) : brightness * (
        waterColor * texWeights[0] +
        grassColor * texWeights[1] +
        rockColor * texWeights[2] +
        snowColor * texWeights[3]
    );
}
