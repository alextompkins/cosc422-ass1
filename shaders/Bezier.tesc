#version 400

layout(vertices = 16) out;
uniform int tessLevel;
uniform int timeSinceExplosion;

vec4 calcPosAtTime(vec4 initialPos, float time, float vy, float vh, float gravity) {
    return vec4(
        initialPos.x + initialPos.x / (abs(initialPos.x) + abs(initialPos.z)) * vh * time,
        initialPos.y + vy * time - 0.5 * gravity * pow(time, 2),
        initialPos.z + initialPos.z / (abs(initialPos.x) + abs(initialPos.z)) * vh * time,
        1
    );
}

void main() {
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }

    if (timeSinceExplosion == 0) {
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    } else {
        float time = timeSinceExplosion * 0.03;

        float initialVel = 15.0;
        float gravity = 5.0;

        vec4 patchPos = vec4(0);
        for (int i = 0; i < 16; i++) {
            patchPos += gl_in[i].gl_Position;
        }
        patchPos /= 16;

        vec4 offset = gl_in[gl_InvocationID].gl_Position - patchPos;

        vec4 initialPos = gl_in[gl_InvocationID].gl_Position;
        vec4 alongGround = vec4(patchPos.x, 0.0, patchPos.z, patchPos.w);
        float theta = acos(dot(normalize(patchPos), normalize(alongGround)));

        float vy = initialVel * sin(theta);
        float vh = initialVel * cos(theta) * 0.5;

        float timeToReachApex = vy / gravity;
        float heightAtApex = calcPosAtTime(patchPos, timeToReachApex, vy, vh, gravity).y;
        float timeFromApexToGround = sqrt(2 * heightAtApex / gravity);
        float totalTime = timeToReachApex + timeFromApexToGround;

        time = min(totalTime, time);

        vec4 newPos = calcPosAtTime(patchPos, time, vy, vh, gravity);

        gl_out[gl_InvocationID].gl_Position = newPos + offset;
    }
}
