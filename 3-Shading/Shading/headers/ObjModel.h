#ifndef OBJMODEL_H
#define OBJMODEL_H

#include<cy/cyTriMesh.h>
#include "model.h"

extern glm::mat4 view, persProjection;
extern glm::vec3 ViewDir, LightPos;
extern float LightIntensity, AmbientIntensity;

class ObjModel : public Model
{
public:
	//---Variables---
	std::string ObjPath;
	cy::TriMesh meshData;
	glm::vec3 Center;
	std::vector<Vertdata> processedData;

	GLuint mvLoc, lightDirLoc, viewDirLoc;
	GLuint diffuseColLoc, lightIntensityLoc, ambientIntensityLoc, shininessLoc;
	glm::mat4 mv;
	glm::mat3 mvNormal;
	glm::vec3 DiffuseColor;
	float Shininess;

	//---Functions---
	ObjModel(std::string pth, std::string shdr, float _shininess);
	void processMesh();

	virtual void initMaterial();
	virtual void updateMaterial();
	virtual void initMVP();
	virtual void updateMVP();
	virtual void SetBuffers();
	virtual void Draw() const;
};

ObjModel::ObjModel(std::string pth, std::string shdr, float _shininess)
{
	ObjPath = pth;
	Shader = shdr;
	Shininess = _shininess;

	if (!meshData.LoadFromFileObj(ObjPath.c_str(), false))
	{
		std::cout << "obj loading failed";
		return;
	}
	std::cout << "obj loading complete \n";
	processMesh();
	compileShaders(Shader + "Vert.glsl", Shader + "Frag.glsl");
	initMaterial();
	initMVP();
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
			glm::vec3 curNormal = glm::vec3(meshData.VN(norm).x, meshData.VN(norm).y, meshData.VN(norm).z);
			//if unassigned
			if (processedData[vert].position == glm::vec3() && processedData[vert].normal == glm::vec3())
			{
				processedData[vert].position = glm::vec3(meshData.V(vert).x, meshData.V(vert).y, meshData.V(vert).z);
				processedData[vert].normal = curNormal;
			}
			else if (processedData[vert].normal != curNormal)
			{
				bool isDealtWith = false;
				if (size > meshData.NV()) //Check in duplicates
				{
					for (int k = meshData.NV() - 1; k < size; k++)
						if (processedData[vert].normal == processedData[k].normal && processedData[vert].position == processedData[k].position)
						{
							meshData.F(i).v[j] = k;
							isDealtWith = true;
						}
				}

				if (!isDealtWith) //Create duplicate vertex
				{
					processedData[size].position = glm::vec3(meshData.V(vert).x, meshData.V(vert).y, meshData.V(vert).z);
					processedData[size].normal = curNormal;
					meshData.F(i).v[j] = size;
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
	//Set values in shader
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
	mvNormal = glm::transpose(glm::inverse(glm::mat3(mv)));
	glUniformMatrix3fv(mvLoc, 1, GL_FALSE, &mvNormal[0][0]);
}

void ObjModel::updateMaterial()
{
	glUseProgram(program);
	glUniform3f(diffuseColLoc, DiffuseColor.x, DiffuseColor.y, DiffuseColor.z);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)offsetof(Vertdata, normal));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ObjModel::Draw() const
{
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, meshData.NF() * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}
#endif // !OBJMODEL_H

