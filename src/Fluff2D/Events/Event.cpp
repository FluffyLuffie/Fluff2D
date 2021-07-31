#include "Event.h"

bool Event::keyDown(int key)
{
	if (glfwGetKey(GLFWWin, key) == GLFW_PRESS)
		return true;
	return false;
}

bool Event::keyPressed(int key)
{
	if (keysCheckFrame.find(key) != keysCheckFrame.end())
		return keysCheckFrame[key];
	return false;
}

bool Event::keyReleased(int key)
{
	if (keysCheckFrame.find(key) != keysCheckFrame.end())
		return !keysCheckFrame[key];
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

void Event::resetKeys()
{
	keysCheckFrame.clear();
}

void Event::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Event::mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
}

void Event::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	scroll = (float)yOffset;
}

void Event::key_callback(GLFWwindow* window, int key, int scanmode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_RELEASE)
		keysCheckFrame[key] = action;
}
