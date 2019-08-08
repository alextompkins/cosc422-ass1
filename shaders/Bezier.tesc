#version 400

layout(vertices = 16) out;
uniform int tessLevel;
uniform int timeSinceExplosion;

vec4 calcPosAtTime(vec4 initialPos, vec4 patchPos, float time, float vy, float vh, float gravity) {
    return vec4(
        initialPos.x + patchPos.x * vh * time,
        initialPos.y + vy * time - 0.5 * gravity * pow(time, 2),
        initialPos.z + patchPos.z * vh * time,
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

        float initialVel = 4.0;
        float gravity = 5.0;

        vec4 patchPos = (
            gl_in[0].gl_Position +
            gl_in[3].gl_Position +
            gl_in[12].gl_Position +
            gl_in[15].gl_Position
        ) * 0.25;

        vec4 initialPos = gl_in[gl_InvocationID].gl_Position;
        vec4 alongGround = vec4(initialPos.x, 0.0, initialPos.z, initialPos.w);
        float theta = acos(dot(normalize(initialPos), normalize(alongGround)));

        float vy = initialVel * sin(theta);
        float vh = initialVel * cos(theta) * 0.5;

        float timeToReachApex = vy / gravity;
        float heightAtApex = calcPosAtTime(initialPos, patchPos, timeToReachApex, vy, vh, gravity).y;
        float timeFromApexToGround = sqrt(2 * heightAtApex / gravity);
        float totalTime = timeToReachApex + timeFromApexToGround;

        time = min(totalTime, time);

        vec4 newPos = calcPosAtTime(initialPos, patchPos, time, vy, vh, gravity);

        gl_out[gl_InvocationID].gl_Position = newPos;
    }
}
