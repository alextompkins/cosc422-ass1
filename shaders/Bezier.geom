#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvMatrix;
uniform mat4 norMatrix;
uniform mat4 mvpMatrix;
uniform vec4 lightPos;
out vec4 primColor;

#define SHININESS 100.0

vec4 white = vec4(1.0);
vec4 grey = vec4(0.2);
vec4 cyan = vec4(0.0, 1.0, 1.0, 1.0);

vec4 calcTriangleNormal(vec3 p0, vec3 p1, vec3 p2) {
    vec3 a = p1 - p0;
    vec3 b = p2 - p0;
    return vec4(normalize(cross(a, b)), 0);
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
        vec4 viewVec = normalize(vec4(-posnEye.xyz, 0));
        vec4 halfVec = normalize(lightVec + viewVec);

        float nDotL = dot(normalEye, lightVec);
        float nDotV = dot(normalEye, viewVec);

        // Compute ambient
        vec4 material = vec4(0.0, 1.0, 1.0, 1.0);
        vec4 ambOut = grey * material;

        // Compute diffuse
        float diffTerm = max(dot(lightVec, normalEye), 0);
        vec4 diffOut = material * diffTerm;

        // Compute specular
        float specTerm = max(dot(halfVec, normalEye), 0);
        vec4 specOut = white * pow(specTerm, SHININESS);

        gl_Position = mvpMatrix * gl_in[i].gl_Position;
        primColor = ambOut + diffOut + specOut;
        EmitVertex();
    }
    EndPrimitive();
}
