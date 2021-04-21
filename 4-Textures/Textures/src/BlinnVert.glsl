#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;

uniform mat4 MVP;
uniform mat3 MV;

out vec3 frag_uv;
out vec3 frag_normal;

void main()
{
	gl_Position = MVP * vec4(pos, 1.0);
	frag_uv = uv;
	frag_normal = normalize(MV * normal);

}