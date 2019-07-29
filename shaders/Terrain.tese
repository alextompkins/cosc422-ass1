#version 400

layout(quads, equal_spacing, ccw) in;

uniform mat4 mvpMatrix;
vec4 posn;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Enter code for posn below.

    // posn = 
    gl_Position = mvpMatrix * posn;
}
