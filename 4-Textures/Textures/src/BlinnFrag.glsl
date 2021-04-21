#version 330 core
in vec3 frag_uv;
in vec3 frag_normal;
in vec3 frag_lightDir;
in vec3 frag_viewDir;
in vec3 frag_diffuseCol;
in float frag_lightIntensity, frag_ambientIntensity, frag_shininess;

vec3 curCol,halfVec;

out vec4 FragCol;

void main()
{
	//Diffuse
	curCol = frag_diffuseCol * frag_lightIntensity * max(0,dot(frag_normal,frag_lightDir));
	//curCol = vec3(1.0f) *  max(0,dot(frag_normal,frag_lightDir));
	
	//Specular
	halfVec = normalize(frag_lightDir + frag_viewDir);	
	curCol += vec3(1.0f) * frag_lightIntensity * pow(max(0,dot(halfVec,frag_normal)), frag_shininess);
	
	//Ambient
	curCol += frag_diffuseCol *frag_ambientIntensity;

	FragCol = vec4(curCol, 1.0f);	
}