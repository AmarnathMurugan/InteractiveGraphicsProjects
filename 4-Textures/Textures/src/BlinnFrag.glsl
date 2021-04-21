#version 330 core

uniform vec3 diffuseCol;
uniform float ambientIntensity;
uniform vec3 lightDir;
uniform vec3 viewDir;
uniform float lightIntensity,shininess;

in vec3 frag_uv;
in vec3 frag_normal;

vec3 curCol,halfVec;

out vec4 FragCol;

void main()
{
	//Diffuse
	curCol = diffuseCol * lightIntensity * max(0,dot(frag_normal,lightDir));
	//curCol = vec3(1.0f) *  max(0,dot(frag_normal,frag_lightDir));
	
	//Specular
	halfVec = normalize(lightDir + viewDir);	
	curCol += vec3(1.0f) * lightIntensity * pow(max(0,dot(halfVec,frag_normal)), shininess);
	
	//Ambient
	curCol += diffuseCol * ambientIntensity;

	FragCol = vec4(curCol, 1.0f);	
}