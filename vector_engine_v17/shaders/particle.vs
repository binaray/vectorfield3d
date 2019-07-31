#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 transform;

void main()
{
	gl_Position = transform * projection * view * model * vec4(aPos, 1.0);
	gl_PointSize = float(1.0);
}