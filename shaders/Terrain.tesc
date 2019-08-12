#version 400

#define NUM_VERTS 4
#define D_MIN 10.0
#define D_MAX 150.0
#define L_LOW 2
#define L_HIGH 20

layout(vertices = 4) out;

uniform vec4 eyePos;

void main() {
    vec4 avgPos = vec4(0);
    for (int i = 0; i < NUM_VERTS; i++) {
        avgPos += gl_in[gl_InvocationID].gl_Position;
    }
    avgPos /= NUM_VERTS;

    float dist = length(avgPos - eyePos);

    float relDistance = (dist - D_MIN) / (D_MAX - D_MIN);
    int tessLevel = int(round(relDistance * (L_LOW - L_HIGH) + L_HIGH));

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
