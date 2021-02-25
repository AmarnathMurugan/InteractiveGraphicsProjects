#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 MVP;
uniform mat3 MV;
uniform vec3 lightDir;
uniform vec3 viewDir;
uniform vec3 diffuseCol;
uniform float lightIntensity,ambientIntensity,shininess;

out vec3 frag_normal;
out vec3 frag_lightDir;
out vec3 frag_viewDir;
out vec3 frag_diffuseCol;
out float frag_lightIntensity, frag_ambientIntensity,frag_shininess;

void main()
{
	gl_Position = MVP * vec4(pos, 1.0);
	frag_normal = normalize(MV * normal);
	frag_lightDir = lightDir;
	frag_viewDir = viewDir;	
	frag_diffuseCol = diffuseCol;
	frag_lightIntensity = lightIntensity;
	frag_ambientIntensity = ambientIntensity;
}