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

    posn = pow(1-u, 2) * (pow(1-v, 2) * P(6) + 2*v*(1-v) * P(7) + pow(v, 2) * P(8))
         + 2*(1-u)*u   * (pow(1-v, 2) * P(3) + 2*v*(1-v) * P(4) + pow(v, 2) * P(5))
         + pow(u, 2)   * (pow(1-v, 2) * P(0) + 2*v*(1-v) * P(1) + pow(v, 2) * P(2));

    gl_Position = mvpMatrix * posn;
}
