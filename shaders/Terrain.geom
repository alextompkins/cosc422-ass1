#version 400

#define TEX_REPEAT_X 5.0
#define TEX_REPEAT_Z 5.0

#define WATER_GRASS_THRESH 2.5
#define GRASS_ROCK_THRESH 5.0
#define ROCK_SNOW_THRESH 7.0

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvMatrix;
uniform mat4 norMatrix;
uniform mat4 mvpMatrix;
uniform vec4 lightPos;

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

    if (height < WATER_GRASS_THRESH) {
        waterWeight = 1;
    } else if (height < GRASS_ROCK_THRESH - 1) {
        grassWeight = 1;
    } else if (height < GRASS_ROCK_THRESH + 1) {
        rockWeight = scaleBetweenThresholds(height, GRASS_ROCK_THRESH - 1, GRASS_ROCK_THRESH + 1);
        grassWeight = 1 - rockWeight;
    } else if (height < ROCK_SNOW_THRESH - 1) {
        rockWeight = 1;
    } else if (height < ROCK_SNOW_THRESH + 1) {
        snowWeight = scaleBetweenThresholds(height, ROCK_SNOW_THRESH - 1, ROCK_SNOW_THRESH + 1);
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

    vec4 normal = calcTriangleNormal(p0, p1, p2);

    // For each of the 3 vertices of each triangle
    for (int i = 0; i < gl_in.length(); i++) {
        vec4 posnEye = mvMatrix * gl_in[i].gl_Position;
        vec4 normalEye = norMatrix * normal;

        vec4 lightVec = normalize(lightPos - posnEye);

        // Compute ambient
        float ambientTerm = 0.1;

        // Compute diffuse
        float nDotL = dot(normalEye, lightVec);
        float diffuseTerm = max(nDotL, 0);

        float s = (mod(gl_in[i].gl_Position.x, TEX_REPEAT_X)) / TEX_REPEAT_X;
        float t = (mod(gl_in[i].gl_Position.z, TEX_REPEAT_Z)) / TEX_REPEAT_Z;
        texCoord = vec2(s, t);

        texWeights = determineWeights(gl_in[i].gl_Position.y);

        gl_Position = mvpMatrix * gl_in[i].gl_Position;
        brightness = min(ambientTerm + diffuseTerm, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
