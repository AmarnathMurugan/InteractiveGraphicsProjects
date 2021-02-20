#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

out vec3 col;
uniform mat4 MVP;
uniform mat3 MV;
void main()
{
	gl_Position = MVP * vec4(pos, 1.0);
	col = MV * normal;
}