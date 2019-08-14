#version 400

#define NUM_VERTS 4
#define D_MIN 10.0
#define D_MAX 150.0
#define L_LOW 2
#define L_HIGH 20

layout(vertices = 4) out;

uniform vec4 eyePos;

int determineTessLevel(float dist) {
    float relDistance = (dist - D_MIN) / (D_MAX - D_MIN);
    int tessLevel = int(round(relDistance * (L_LOW - L_HIGH) + L_HIGH));
    if (tessLevel < L_LOW) tessLevel = L_LOW;
    else if (tessLevel > L_HIGH) tessLevel = L_HIGH;
    return tessLevel;
}

void main() {
    if (gl_InvocationID == 0) {
        // Calculate outer tess levels
        vec4 start, end, avgPos;
        for (int i = 0; i < 4; i++) {
            start = gl_in[int(mod(i - 1, 4))].gl_Position;
            end = gl_in[i].gl_Position;
            avgPos = (start + end) / 2;

            gl_TessLevelOuter[i] = determineTessLevel(length(avgPos - eyePos));
        }

        // Calculate inner tess levels
        vec4 avgPatchPos = vec4(0);
        for (int i = 0; i < NUM_VERTS; i++) {
            avgPatchPos += gl_in[gl_InvocationID].gl_Position;
        }
        avgPatchPos /= NUM_VERTS;

        int innerLevel = determineTessLevel(length(avgPatchPos - eyePos));
        gl_TessLevelInner[0] = innerLevel;
        gl_TessLevelInner[1] = innerLevel;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
