#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<cy/cyTriMesh.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "headers/cutils.h"

void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseInputCallback(GLFWwindow* window, int key, int action, int mods);
void framebufferResizeCallback(GLFWwindow* window, int w, int h);
void initMVP();
void updateMVP();
void CompileShaders();

void mouseInputTransformations(GLFWwindow* window);

unsigned int width = 960, height = 540;
float yRot=0, xRot=0,dist=50,yOffset=0;
glm::mat4 persProjection, orthoProjection, model, view, mvp;
glm::dvec2 prevMousePosL, prevMousePosR, curMousePosL, deltaMousePosL, curMousePosR;
glm::vec3 UpAxis(0.0f, 1.0f, 0.0f), RightAxis(1.0f, 0.0f, 0.0f), Center;
bool isPerspective=true,isLeftMouseHeld = false, isRightMouseHeld = false, isRecompile=false;
GLuint mvpLocation;
GLuint program;

int main(int argc, char* argv[])
{
		
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
	glfwSetMouseButtonCallback(window, mouseInputCallback);
	glfwSetKeyCallback(window, inputCallback);	

	//Load model
	cy::TriMesh meshData;
	if (!meshData.LoadFromFileObj(argv[1], false))
	{
		std::cout << "obj loading failed";
		return -1;
	}	
	std::cout << "obj loading complete";		
	meshData.ComputeBoundingBox();
	if (meshData.IsBoundBoxReady())
	{		
		cy::Vec3f center = (meshData.GetBoundMin() + meshData.GetBoundMax())/2.0f;
		Center = glm::vec3 (center.x, center.y, center.z);	
	}

	//Set program
	CompileShaders();	
	glUseProgram(program);	

	//Set MVP matrix
	initMVP();	
	mvpLocation = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);

	//Set buffers
	GLuint VBO,VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*meshData.NV(), &meshData.V(0), GL_STATIC_DRAW);	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (isRightMouseHeld || isLeftMouseHeld) mouseInputTransformations(window);

		if (isRecompile)
		{
			glDeleteProgram(program);
			CompileShaders();
			glUseProgram(program);
			mvpLocation = glGetUniformLocation(program, "MVP");
			glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
			isRecompile = false;			
		}
		else
			glUseProgram(program);

		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, meshData.NV());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(program);
	glfwTerminate();
	return 0;
}

void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);	

	if(key==GLFW_KEY_P && action == GLFW_PRESS)
	{
		isPerspective = !isPerspective;
		updateMVP();
	}

	if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
		isRecompile = true;
}

void mouseInputCallback(GLFWwindow* window, int key, int action, int mods)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			isLeftMouseHeld = true;
			glfwGetCursorPos(window, &prevMousePosL.x, &prevMousePosL.y);
		}
		else if (action == GLFW_RELEASE)
			isLeftMouseHeld = false;
	}

	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &prevMousePosR.x, &prevMousePosR.y);
			isRightMouseHeld = true;
		}
		else if (action == GLFW_RELEASE)
			isRightMouseHeld = false;
	}
}

void framebufferResizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	updateMVP();
}

void CompileShaders()
{
	GLuint vertShader, fragShader;

	//Read Vert Shader file and compile
	std::string vertStr = GetStringFromFile("src/vert.glsl");
	const char* vert = vertStr.c_str();

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vert, NULL);
	glCompileShader(vertShader);
	int success;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::cout << "Vert compilation failed";
		return;
	}
	
	//Read Frag Shader file and compile
	std::string fragStr = GetStringFromFile("src/frag.glsl");
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
	if (!success)
	{
		std::cout << "Link failed";
		return;
	}
	std::cout << "Shader Compilation Complete \n";
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void initMVP()
{
	persProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.0f, 500.0f);
	view = glm::lookAt(glm::vec3(0, -50.0f,dist), glm::vec3(0), UpAxis);
	model = glm::translate(glm::mat4(1.0f), -Center);
	mvp = persProjection * view * model;
}

void updateMVP()
{
	//Updates perspective when window size changes
	persProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 200.0f);
	//Changes to cam pos through input
	view = glm::lookAt(glm::vec3(0, -50.0f, dist), glm::vec3(0), UpAxis);
	if (!isPerspective)
	{
		orthoProjection = glm::ortho(-50.0f * (float)width / (float)height, 50.0f * (float)width / (float)height, -50.0f, 50.0f, 0.1f, 100.0f);		
		mvp = orthoProjection * view * model;
	}
	else	
		mvp = persProjection * view * model;
	
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
}

void mouseInputTransformations(GLFWwindow* window)
{
	double delta = 0;
	if (isRightMouseHeld)
	{
		glfwGetCursorPos(window, &curMousePosR.x, &curMousePosR.y);
		delta = curMousePosR.y - prevMousePosR.y;
		dist += delta;		
		dist = dist < 0.5 ? 0.5 : dist;
		dist = dist > 80 ? 80 : dist;
		glfwGetCursorPos(window, &prevMousePosR.x, &prevMousePosR.y);
	}
	if (isLeftMouseHeld)
	{
		glfwGetCursorPos(window, &curMousePosL.x, &curMousePosL.y);
		deltaMousePosL = curMousePosL - prevMousePosL;
		model = glm::rotate(model,(float)deltaMousePosL.x * 0.01f, glm::vec3(0.0f,0.0f,1.0f));
		/*model = glm::translate(model, Center);
		model = glm::rotate(model, (float)deltaMousePosL.y * 0.01f, RightAxis);
		model = glm::translate(model, -Center);*/
		glfwGetCursorPos(window, &prevMousePosL.x, &prevMousePosL.y);
	}
	updateMVP();
}