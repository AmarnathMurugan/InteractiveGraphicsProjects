#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<cy/cyTriMesh.h>
#include <map>
#include <chrono>

#include "headers/cutils.h"
#include "headers/ProceduralModel.h"
#include "headers/ObjModel.h"

//OpenGL callbacks
void inputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseInputCallback(GLFWwindow* window, int key, int action, int mods);
void framebufferResizeCallback(GLFWwindow* window, int w, int h);


void initMVP();
void updateMVP();
void initOctahedronBuffer();
void mouseInputTransformations(GLFWwindow* window);

unsigned int width = 960, height = 540;

float CamSpeed = 0.1f,camDist=0, lightScale = 0.2;
glm::mat4 persProjection, orthoProjection, view, vectorRotationMatrix(1.0f);
glm::dvec2 prevMousePos, curMousePos, deltaMousePos;
glm::vec2 CamDistLimit(0.5f, 5.0f);

//Material properties
glm::vec3 LightPos(0.0f, 1.5f, 0.0f), ViewDir(0,3,-3), DiffuseColor(0.2f, 0.8f, 0.7f);
float LightIntensity = 1.0f, AmbientIntensity = 0.1f, Shininess = 50.0f;

bool isPerspective=true, isLeftMouseHeld = false, isRightMouseHeld = false, isCtrlHeld = false, isRecompile=false;

std::vector<std::shared_ptr<Model>> World;

int main(int argc, char* argv[])
{
	//Init GLFW and Versions
	if (!glfwInit())	
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	//Create window
	GLFWwindow* window = glfwCreateWindow(width, height, "Shading", NULL, NULL);
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
	glEnable(GL_DEPTH_TEST);

	initMVP();
	
	initOctahedronBuffer();
	World.emplace_back(std::make_shared<ObjModel>(argv[1], "Blinn", glm::vec3(0.2f, 0.8f, 0.7f), Shininess, true));

	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (isRightMouseHeld || isLeftMouseHeld) mouseInputTransformations(window);		
		
		if (isRecompile)
		{
			for (int i = 0; i < World.size(); i++)
				World[i]->RecompileShaders();
			isRecompile = false;			
		}
		
		for (int i = 0; i < World.size(); i++)
			World[i]->Draw();		

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

	if(key==GLFW_KEY_P && action == GLFW_PRESS)
	{
		isPerspective = !isPerspective;
		updateMVP();
	}

	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			isCtrlHeld = true;
			glfwGetCursorPos(window, &prevMousePos.x, &prevMousePos.y);
		}
		else if (action == GLFW_RELEASE)
			isCtrlHeld = false;
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
			glfwGetCursorPos(window, &prevMousePos.x, &prevMousePos.y);
		}
		else if (action == GLFW_RELEASE)
			isLeftMouseHeld = false;
	}

	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &prevMousePos.x, &prevMousePos.y);
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

void initMVP()
{
	camDist = ViewDir.length();
	ViewDir = glm::normalize(ViewDir);
	persProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.5f, 10.0f);
	view = glm::lookAt(ViewDir*camDist, glm::vec3(0.0), UpAxis);	
}

void updateMVP()
{
	//Updates perspective when window size changes
	persProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.5f, 10.0f);
	//Changes to cam pos through input
	view = glm::lookAt(ViewDir * camDist, glm::vec3(0.0), UpAxis);
	if (!isPerspective)
	{
		float multiplier = 5.0f;
		orthoProjection = glm::ortho(-multiplier * (float)width / (float)height, multiplier * (float)width / (float)height, -multiplier, multiplier, 0.1f, 10.0f);		
		
	}
	for (int i = 0; i < World.size(); i++)
		World[i]->updateMVP();
}

void mouseInputTransformations(GLFWwindow* window)
{
	glfwGetCursorPos(window, &curMousePos.x, &curMousePos.y);
	deltaMousePos = curMousePos - prevMousePos;
	if (isRightMouseHeld)
	{ 
		if (isCtrlHeld)
		{
			vectorRotationMatrix = glm::rotate(glm::mat4(1.0), (float)deltaMousePos.y * 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));		
			LightPos = glm::vec3((vectorRotationMatrix * glm::vec4(LightPos, 1.0f)));			
		}
		else
		{
			camDist += deltaMousePos.y * CamSpeed;
			camDist = camDist < CamDistLimit.x ? CamDistLimit.x : camDist;
			camDist = camDist > CamDistLimit.y ? CamDistLimit.y : camDist;
		}
	}
	if (isLeftMouseHeld)
	{
		vectorRotationMatrix = glm::rotate(glm::mat4(1.0), (float)deltaMousePos.x * -0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		if (isCtrlHeld)
		{
			LightPos = glm::vec3((vectorRotationMatrix * glm::vec4(LightPos,1.0f)));
		}
		else
		{			
			ViewDir = glm::vec3((vectorRotationMatrix * glm::vec4(ViewDir, 0.0f)));
			vectorRotationMatrix = glm::rotate(glm::mat4(1.0), (float)deltaMousePos.y * 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));
			ViewDir = glm::vec3((vectorRotationMatrix * glm::vec4(ViewDir, 0.0f)));
		}
	}
	prevMousePos = curMousePos;
	updateMVP();
	for (int i = 0; i < World.size(); i++)
		World[i]->updateMaterial();
}

void initOctahedronBuffer()
{
	float TetraPos[] =
	{
		0,0.75,0,
		-0.5,0,0.5,
		0.5,0,0.5,
		0.5,0,-0.5,
		-0.5,0,-0.5,
		0,-0.75,0
	};

	unsigned int TetraElems[] =
	{
		0,4,1,
		0,1,2,
		0,2,3,
		0,3,4,
		5,4,3,
		5,1,4,
		5,1,2,
		5,3,2
	};

	World.emplace_back(std::make_unique<ProceduralModel>(TetraPos, sizeof(TetraPos), TetraElems, sizeof(TetraElems), "Unlit", lightScale));
}