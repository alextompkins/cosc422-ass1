#version 400

layout(quads, equal_spacing, ccw) in;

uniform mat4 mvpMatrix;
vec4 posn;

vec4 P(int pos) {
    return gl_in[pos].gl_Position;
}

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    posn = (1 - u) * (1 - v) * P(0)
         + u * (1 - v)       * P(1)
         + u * v             * P(2)
         + (1 - u) * v       * P(3);

    gl_Position = mvpMatrix * posn;
}
