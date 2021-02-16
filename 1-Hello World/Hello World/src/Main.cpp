#include <GLFW/glfw3.h>
#include<stdlib.h>
#include<time.h>
#include "headers/cutils.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

double currentTime = 0;

void main()
{	
	srand(time(NULL));
	Color prevColor = GetRandomColor();
	Color currentColor = prevColor;
	Color targetColor = GetRandomColor();
	

	if (!glfwInit())	
		return;

	GLFWwindow* window = glfwCreateWindow(960, 540, "New Window", NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
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
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);	
}

