#include "Event.h"

bool Event::keyPressed(int key)
{
	if (glfwGetKey(GLFWWin, key) == GLFW_PRESS)
		return true;
	return false;
}

float Event::getScroll()
{
	float temp = scroll;
	scroll = 0.0f;
	return temp;
}

void Event::calculateDeltaTime()
{
	float currentTime = static_cast<float>(glfwGetTime());
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;
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
	scroll = (float)yOffset;
}
