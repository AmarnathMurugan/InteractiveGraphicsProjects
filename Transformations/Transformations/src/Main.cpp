#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stdlib.h>
#include<time.h>
#include "headers/cutils.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

double currentTime = 0;

int main()
{	
	srand(time(NULL));
	Color prevColor = GetRandomColor();
	Color currentColor = prevColor;
	Color targetColor = GetRandomColor();
	

	if (!glfwInit())	
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(960, 540, "New Window", NULL, NULL);
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

	glfwSetKeyCallback(window, keyCallback);

	if(!glfwWindowShouldClose(window))
		glClearColor(currentColor.r(),currentColor.g(),currentColor.b(), 1.0);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);

		float interpolant = glfwGetTime() - currentTime;
		lerpColor(prevColor, targetColor, interpolant/2.0, currentColor);
		glClearColor(currentColor.r(), currentColor.g(), currentColor.b(), 1.0);

		if (interpolant > 2.0)
		{
			prevColor = currentColor;
			targetColor = GetRandomColor();
			currentTime = glfwGetTime();
		}

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);	
}

