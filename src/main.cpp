#include "main.hpp"


#define GLFW_STATIC
#include <GLFW/glfw3.h>


void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);


int main(int in_varc, char** in_varv)
{
	if (!glfwInit())
	{
		std::cerr << "ERROR: couldnt init glfw." << std::endl;
		return 1;
	}

	// tell glfw we want to use core profile(no legacy stuff)
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	// NO API since we wont use opengl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
	if (!window)
	{
		std::cerr << "ERROR: couldnt create window." << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);

	while (!glfwWindowShouldClose(window))
	{



		// get the events
		glfwPollEvents();
		processInput(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	
}

void processInput(GLFWwindow* window)
{
	/*
	Ask glfw is key was pressed and do stuff acorrding
	*/
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

}