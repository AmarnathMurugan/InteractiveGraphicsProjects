#version 330 core

layout(location = 0) in vec3 pos;

void main()
{
	gl_Position =  vec4(pos*0.05, 1.0);
}