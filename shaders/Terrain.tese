#version 400

#define X_MIN -45.0
#define X_MAX 45.0
#define Z_MIN 0.0
#define Z_MAX -90.0
#define HEIGHT_SCALE 10.0

layout(quads, equal_spacing, ccw) in;

uniform sampler2D heightMapper;
uniform mat4 mvpMatrix;

vec4 posn;

vec4 P(int pos) {
    return gl_in[pos].gl_Position;
}

void main() {
    // Bi-linear mapping
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    posn = (1 - u) * (1 - v) * P(0)
         + u * (1 - v)       * P(1)
         + u * v             * P(2)
         + (1 - u) * v       * P(3);

    // Height mapping
    float s = (posn.x - X_MIN) / (X_MAX - X_MIN);
    float t = (posn.z - Z_MIN) / (Z_MAX - Z_MIN);
    float height = texture(heightMapper, vec2(s, t)).x;
    posn.y = height * HEIGHT_SCALE;

    // Transform to clip co-ords
    gl_Position = posn;
}
