#ifndef PROCEDURALMODEL_H
#define PROCEDURALMODEL_H

#include "model.h"

extern glm::mat4 view, persProjection;
extern glm::vec3 LightPos;
class ProceduralModel : public Model
{
	public:
		glm::vec3 scale;
		float *vertPos;
		unsigned int* indices;
		int vertPosArrSize, indicesArrSize;
	public:
		ProceduralModel(float* _vertPos, int _vertPosArrSize, unsigned int* _indices,int _indicesArrSize, std::string shdr, float _scale);
		virtual void initMVP();
		virtual void initMaterial();
		virtual void updateMaterial();
		virtual void updateMVP();
		virtual void SetBuffers();
		virtual void Draw() const;
};

ProceduralModel::ProceduralModel(float* _vertPos, int _vertPosArrSize, unsigned int* _indices, int _indicesArrSize, std::string shdr, float _scale)
{
	vertPos = _vertPos;
	vertPosArrSize = _vertPosArrSize;
	indices = _indices;
	indicesArrSize = _indicesArrSize;
	Shader = shdr;
	scale = glm::vec3(_scale);
	compileShaders(Shader + "Vert.glsl", Shader + "Frag.glsl");	
	initMaterial();
	initMVP();
	updateMaterial();
	SetBuffers();
}

void ProceduralModel::initMVP()
{	
	updateMVP();
}

void ProceduralModel::updateMVP()
{		
	modelMat = glm::scale(glm::mat4(1.0f), scale);
	modelMat = glm::translate(modelMat, LightPos / scale);
	mvp = persProjection * view * modelMat;	
	glUseProgram(program);
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
}

void ProceduralModel::SetBuffers()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertPosArrSize, vertPos, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArrSize, indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

void ProceduralModel::Draw() const
{
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, vertPosArrSize, GL_UNSIGNED_INT, 0);
}

void ProceduralModel::initMaterial()
{
	mvpLoc = glGetUniformLocation(program, "MVP");
}

void ProceduralModel::updateMaterial()
{
	return;
}

#endif // !PROCEDURALMODEL_H

