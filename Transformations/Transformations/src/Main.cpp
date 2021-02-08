#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<cy/cyTriMesh.h>
#include<stdlib.h>
#include<time.h>
#include "headers/cutils.h"

void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferResizeCallback(GLFWwindow* window, int w, int h);
//void GenerateVertexBuffer();
GLuint CompileShaders();
unsigned int width = 960, height = 540;

int main(int argc, char* argv[])
{	
	//Set Background Colors
	srand(time(NULL));
	Color prevColor = GetRandomColor();
	Color currentColor = prevColor;
	Color targetColor = GetRandomColor();
	
	//Init GLFW and Versions
	if (!glfwInit())	
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create window
	GLFWwindow* window = glfwCreateWindow(width, height, "New Window", NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Unable to load GLAD";
		return -1;
	}

	//Set Callbacks
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetKeyCallback(window, inputCallback);	

	//Load model
	cy::TriMesh meshData;
	if (!meshData.LoadFromFileObj(argv[1], false))
	{
		std::cout << "obj loading failed";
		return -1;
	}	
	std::cout << "obj loading complete, V:("<< meshData.V(0).elem[0]<<","<< meshData.V(0).elem[1]<<","<< meshData.V(0).elem[2]<<")";
	
	GLuint program = CompileShaders();

	
	float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
	};
	unsigned int VBO,VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferData(VBO, sizeof(cy::Vec3f)*meshData.NV(), &meshData.V(0), GL_STATIC_DRAW);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
	
	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		//Set next target
	
		glUseProgram(program);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);	
}

void framebufferResizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

GLuint CompileShaders()
{
	GLuint vertShader, fragShader;
	vertShader = glCreateShader(GL_VERTEX_SHADER);

	std::string vertStr = GetStringFromFile("src/vert.glsl");
	const char* vert = vertStr.c_str();
	glShaderSource(vertShader, 1, &vert, NULL);
	glCompileShader(vertShader);
	int success;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout << "Vert compilation failed";
		return 0;
	}

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragStr = GetStringFromFile("src/frag.glsl");
	const char* frag = fragStr.c_str();

	glShaderSource(fragShader, 1, &frag, NULL);

	

	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout << "Frag compilation failed";
		return 0;
	}
	
	GLuint program;
	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		std::cout << "Link failed";
		return 0;
	}
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	return program;
}