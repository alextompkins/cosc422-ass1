#version 400

#define X_MIN -45.0
#define X_MAX 45.0
#define Z_MIN 0.0
#define Z_MAX -100.0
#define TEXTURE_REPEATS 8.0
#define MAX_DEPTH 5.0

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvMatrix;
uniform mat4 norMatrix;
uniform mat4 mvpMatrix;
uniform vec4 lightPos;
uniform float waterLevel;
uniform float snowLevel;
float rockLevel = snowLevel - 2.0;

out float brightness;
out vec4 texWeights;
out vec2 texCoord;

vec4 calcTriangleNormal(vec3 p0, vec3 p1, vec3 p2) {
    vec3 a = p1 - p0;
    vec3 b = p2 - p0;
    return vec4(normalize(cross(a, b)), 0);
}

float scaleBetweenThresholds(float value, float lowThresh, float highThresh) {
    return (value - lowThresh) / (highThresh - lowThresh);
}

vec4 determineWeights(float height) {
    float waterWeight = 0;
    float grassWeight = 0;
    float rockWeight = 0;
    float snowWeight = 0;

    if (height < waterLevel) {
        waterWeight = 1;
    } else if (height < rockLevel - 0.5) {
        grassWeight = 1;
    } else if (height < rockLevel + 0.5) {
        rockWeight = scaleBetweenThresholds(height, rockLevel - 2.5, rockLevel + 2.5);
        grassWeight = 1 - rockWeight;
    } else if (height < snowLevel - 0.5) {
        rockWeight = 1;
    } else if (height < snowLevel + 0.5) {
        snowWeight = scaleBetweenThresholds(height, snowLevel - 0.5, snowLevel + 0.5);
        rockWeight = 1 - snowWeight;
    } else {
        snowWeight = 1;
    }

    return vec4(waterWeight, grassWeight, rockWeight, snowWeight);
}

void main() {
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec4 modifiedPos[3];
    for (int i = 0; i < gl_in.length(); i++) {
        modifiedPos[i] = gl_in[i].gl_Position;
        modifiedPos[i].y = max(modifiedPos[i].y, waterLevel);
    }
    vec4 normal = calcTriangleNormal(modifiedPos[0].xyz, modifiedPos[1].xyz, modifiedPos[2].xyz);

    // For each of the 3 vertices of each triangle
    for (int i = 0; i < gl_in.length(); i++) {
        vec4 origPos = gl_in[i].gl_Position;
        vec4 newPos = modifiedPos[i];

        float s = (newPos.x - X_MIN) / (X_MAX - X_MIN) * TEXTURE_REPEATS;
        float t = (newPos.z - Z_MIN) / (Z_MAX - Z_MIN) * TEXTURE_REPEATS;
        texCoord = vec2(s, t);
        texWeights = determineWeights(origPos.y);

        vec4 posnEye = mvMatrix * newPos;
        vec4 normalEye = norMatrix * normal;
        vec4 lightVec = normalize(lightPos - posnEye);

        // Compute ambient
        float ambientTerm = 0.1;

        // Compute diffuse
        float nDotL = dot(normalEye, lightVec);
        float diffuseTerm = max(nDotL, 0);

        // Compute depth lighting
        float depth = newPos.y - origPos.y;
        float depthTerm = min(depth / MAX_DEPTH, 0.4);

        gl_Position = mvpMatrix * newPos;
        brightness = min(ambientTerm + diffuseTerm - depthTerm, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
