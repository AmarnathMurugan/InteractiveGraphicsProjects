#version 330 core

uniform vec3 diffuseCol;
uniform sampler2D mainTex;
uniform sampler2D specTex;
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
	curCol = diffuseCol * texture2D(mainTex,frag_uv.xy).xyz *  max(0,dot(frag_normal,lightDir));
	
	//Specular
	halfVec = normalize(lightDir + viewDir);	
	curCol += texture2D(specTex,frag_uv.xy).xyz * lightIntensity * pow(max(0,dot(halfVec,frag_normal)), shininess);
	
	//Ambient
	curCol += diffuseCol * texture2D(mainTex,frag_uv.xy).xyz * ambientIntensity;
	FragCol = vec4(curCol, 1.0f);
}