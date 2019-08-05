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

    float Au = pow(1 - u, 3);
    float Bu = 3 * u * pow(1 - u, 2);
    float Cu = 3 * pow(u, 2) * (1 - u);
    float Du = pow(u, 3);
    float Av = pow(1 - v, 3);
    float Bv = 3 * v * pow(1 - v, 2);
    float Cv = 3 * pow(v, 2) * (1 - v);
    float Dv = pow(v, 3);

    posn = Au * (Av * P(0) + Bv * P(1) + Cv * P(2) + Dv * P(3))
         + Bu * (Av * P(4) + Bv * P(5) + Cv * P(6) + Dv * P(7))
         + Cu * (Av * P(8) + Bv * P(9) + Cv * P(10) + Dv * P(11))
         + Du * (Av * P(12) + Bv * P(13) + Cv * P(14) + Dv * P(15));

    gl_Position = posn;
}
