#include "Event.h"

Event::Event()
{
}

Event::~Event()
{
}

void Event::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Event::processInput(GLFWwindow* window)
{
	//testing, do something else later, maybe a switch statement
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void Event::mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
}

void Event::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
}
