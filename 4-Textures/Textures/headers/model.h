#ifndef MODEL_H
#define MODEL_H

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <string>
#include "headers/cutils.h"

struct Vertdata
{
	glm::vec3 position, normal, uv;
};

class Model
{
	public:
		//---Variables---
		unsigned int vao, vbo, ebo, program;
		GLuint mvpLoc;
		glm::mat4 modelMat, mvp;
		std::string Shader;

		//---Functions---
		void compileShaders(std::string vertShdrName, std::string fragShdrName);
		void RecompileShaders();

		virtual void initMVP() = 0;
		virtual void updateMVP() = 0;
		virtual void initMaterial() = 0;
		virtual void updateMaterial() = 0;
		virtual void SetBuffers() = 0;
		virtual void Draw() const = 0;
		
		~Model();
};


void Model::compileShaders(std::string vertShdrName, std::string fragShdrName)
{
	GLuint vertShader, fragShader;
	//Read Vert Shader file and compile
	std::string path = "src/";
	std::string vertStr = GetStringFromFile((path + vertShdrName).c_str());
	const char* vert = vertStr.c_str();
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vert, NULL);
	glCompileShader(vertShader);
	int success;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout <<Shader<< "Vert compilation failed";
		return;
	}

	//Read Frag Shader file and compile
	std::string fragStr = GetStringFromFile((path + fragShdrName).c_str());
	const char* frag = fragStr.c_str();
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &frag, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout << "Frag compilation failed";
		return;
	}

	//Create program and link 
	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	char infoLog[512];
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "Linking Failed:" << infoLog << std::endl;
	}
	std::cout << "Shader Compilation Complete \n";
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void Model::RecompileShaders()
{
	glDeleteProgram(program);
	compileShaders(Shader + "Vert.glsl", Shader + "Frag.glsl");
	glUseProgram(program);
	initMaterial();
	updateMVP();
	updateMaterial();
	std::cout << "Shader recompile done";
}

Model::~Model()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(program);
}

#endif // !MODEL_H

