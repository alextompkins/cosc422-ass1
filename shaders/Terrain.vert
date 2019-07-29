#version 330

layout (location = 0) in vec4 position;

uniform mat4 mvpMatrix;
out vec4 gl_Position;


void main()
{
   gl_Position = mvpMatrix * position;
}
