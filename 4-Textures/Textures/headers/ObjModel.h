#ifndef OBJMODEL_H
#define OBJMODEL_H

#include<cy/cyTriMesh.h>
#include "model.h"
#include "picopng.h"

extern glm::mat4 view, persProjection;
extern glm::vec3 ViewDir, LightPos;
extern float LightIntensity, AmbientIntensity;

class ObjModel : public Model
{
public:
	//---Variables---
	std::string ObjPath,assetDir;
	cy::TriMesh meshData;
	glm::vec3 Center, DiffuseColor;
	std::vector<Vertdata> processedData;

	std::vector<unsigned char> imgBuffer, imgTex;	

	GLuint mvLoc, lightDirLoc, viewDirLoc, texture;
	GLuint diffuseColLoc, diffuseTexLoc, lightIntensityLoc, ambientIntensityLoc, shininessLoc;
	glm::mat4 mv;
	glm::mat3 mvNormal;
	float Shininess;

	//---Functions---
	ObjModel(std::string pth, std::string shdr, glm::vec3 diff, float _shininess, bool loadMtl);
	void processMesh();
	void loadTextures();

	virtual void initMaterial();
	virtual void updateMaterial();
	virtual void initMVP();
	virtual void updateMVP();
	virtual void SetBuffers();
	virtual void Draw() const;

};

ObjModel::ObjModel(std::string pth, std::string shdr, glm::vec3 diff, float _shininess, bool loadMtl = false)
{
	ObjPath = pth;
	int indx = ObjPath.find_last_of('\\') + 1;
	assetDir = ObjPath.substr(0, indx);
	Shader = shdr;
	Shininess = _shininess;
	DiffuseColor = diff;	

	if (!meshData.LoadFromFileObj(ObjPath.c_str(), loadMtl))
	{
		std::cout << "obj loading failed";
		return;
	}
	processMesh();
	compileShaders(Shader + "Vert.glsl", Shader + "Frag.glsl");
	initMaterial();
	initMVP();
	loadTextures();
	updateMaterial();
	SetBuffers();
}

void ObjModel::processMesh()
{
	meshData.ComputeBoundingBox();
	if (meshData.IsBoundBoxReady())
	{
		cy::Vec3f center = (meshData.GetBoundMin() + meshData.GetBoundMax()) / 2.0f;
		Center = glm::vec3(center.x, center.y, center.z);
	}

	processedData.resize((int)(meshData.NV() * 1.1));
	int size = meshData.NV();
	for (int i = 0; i < meshData.NF(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int vert = meshData.F(i).v[j];
			int norm = meshData.FN(i).v[j];
			int uv = meshData.FT(i).v[j];
			glm::vec3 curNormal = glm::vec3(meshData.VN(norm).x, meshData.VN(norm).y, meshData.VN(norm).z);
			glm::vec3 curUV = glm::vec3(meshData.VT(uv).x, meshData.VT(uv).y, meshData.VT(uv).z);
			//if unassigned
			if (processedData[vert].position == glm::vec3() && processedData[vert].normal == glm::vec3() && processedData[vert].uv == glm::vec3())
			{
				processedData[vert].position = glm::vec3(meshData.V(vert).x, meshData.V(vert).y, meshData.V(vert).z);
				processedData[vert].normal = curNormal;
				processedData[vert].uv = curUV;
			}
			else if (processedData[vert].normal != curNormal && processedData[vert].uv != curUV)
			{
				bool isDealtWith = false;
				if (size > meshData.NV()) //Check in duplicates
				{
					for (int k = meshData.NV() - 1; k < size; k++)
						if (processedData[vert].normal == processedData[k].normal && processedData[vert].position == processedData[k].position && processedData[vert].uv == processedData[k].uv)
						{
							meshData.F(i).v[j] = k;
							meshData.FT(i).v[j] = k;
							isDealtWith = true;
						}
				}

				if (!isDealtWith) //Create duplicate vertex
				{
					processedData[size].position = glm::vec3(meshData.V(vert).x, meshData.V(vert).y, meshData.V(vert).z);
					processedData[size].normal = curNormal;
					processedData[size].uv = curUV;
					meshData.F(i).v[j] = size;
					meshData.FT(i).v[j] = size;
					size++;
				}
			}
		}
	}
}

void ObjModel::initMaterial()
{
	mvpLoc = glGetUniformLocation(program, "MVP");
	mvLoc = glGetUniformLocation(program, "MV");
	lightDirLoc = glGetUniformLocation(program, "lightDir");
	viewDirLoc = glGetUniformLocation(program, "viewDir");
	diffuseColLoc = glGetUniformLocation(program, "diffuseCol");
	diffuseTexLoc = glGetUniformLocation(program, "mainTex");
	lightIntensityLoc = glGetUniformLocation(program, "lightIntensity");
	ambientIntensityLoc = glGetUniformLocation(program, "ambientIntensity");
	shininessLoc = glGetUniformLocation(program, "shininess");
}

void ObjModel::initMVP()
{	
	//Scale model to make height 1, rotate to make it upright and center model to origin
	modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / (meshData.GetBoundMax().y - meshData.GetBoundMin().y)));
	modelMat = glm::rotate(modelMat, glm::radians(-90.0f), RightAxis);
	modelMat = glm::translate(modelMat, glm::vec3(0.0, 0.0, -0.5f * (meshData.GetBoundMax().z - meshData.GetBoundMin().z)));
	updateMVP();
}

void ObjModel::updateMVP()
{
	glUseProgram(program);
	mv = view * modelMat;
	mvp = persProjection * mv;
	mvNormal = glm::transpose(glm::inverse(glm::mat3(mv)));
	//Set values in shader
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix3fv(mvLoc, 1, GL_FALSE, &mvNormal[0][0]);
}

void ObjModel::loadTextures()
{
	if (meshData.NM() == 0) return;
	if (std::string(meshData.M(0).map_Kd).length() < 1) return;
	unsigned long w, h;
	std::cout <<"\nName:" << std::string(meshData.M(0).map_Kd);
	loadFile(imgBuffer, assetDir + std::string(meshData.M(0).map_Kd));
	int error = decodePNG(imgTex, w, h, imgBuffer.empty() ? 0 : &imgBuffer[0], (unsigned long)imgBuffer.size());
	if (error != 0) std::cout << "error: " << error << std::endl;
	if (imgTex.size() > 4) std::cout << "width: " << w << " height: " << h << " first pixel: "  << int(imgTex[0])<<"," << int(imgTex[1])<<"," << int(imgTex[2])<<"," << int(imgTex[3]) << std::endl;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &imgTex[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void ObjModel::updateMaterial()
{
	glUseProgram(program);
	glUniform3f(diffuseColLoc, DiffuseColor.x, DiffuseColor.y, DiffuseColor.z);
	glUniform1i(diffuseTexLoc, 0);
	glm::vec3 viewSpaceVec = glm::vec3(view * glm::vec4(ViewDir, 0.0f));
	glUniform3f(viewDirLoc, viewSpaceVec.x, viewSpaceVec.y, viewSpaceVec.z);;
	viewSpaceVec = glm::vec3(view * glm::vec4(glm::normalize(LightPos), 0.0f));
	glUniform3f(lightDirLoc, viewSpaceVec.x, viewSpaceVec.y, viewSpaceVec.z);
	glUniform1f(lightIntensityLoc, LightIntensity);
	glUniform1f(ambientIntensityLoc, AmbientIntensity);
	glUniform1f(shininessLoc, Shininess);
	glUseProgram(0);
}

void ObjModel::SetBuffers()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertdata) * processedData.size(), &processedData[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cy::TriMesh::TriFace) * meshData.NF(), &meshData.F(0), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertdata), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertdata), (void*)offsetof(Vertdata, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertdata), (void*)offsetof(Vertdata, uv));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ObjModel::Draw() const
{
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, meshData.NF() * 3, GL_UNSIGNED_INT, 0);
}
#endif // !OBJMODEL_H

