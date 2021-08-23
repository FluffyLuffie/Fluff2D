#include "Event.h"

bool Event::keyDown(int key)
{
	if (!io->WantCaptureKeyboard && glfwGetKey(GLFWWin, key) == GLFW_PRESS)
		return true;
	return false;
}

bool Event::keyPressed(int key)
{
	if (!io->WantCaptureKeyboard && keysCheckFrame.find(key) != keysCheckFrame.end())
		return keysCheckFrame[key];
	return false;
}

bool Event::keyReleased(int key)
{
	if (!io->WantCaptureKeyboard && keysCheckFrame.find(key) != keysCheckFrame.end())
		return !keysCheckFrame[key];
	return false;
}

float Event::getScroll()
{
	if (!io->WantCaptureMouse)
		return io->MouseWheel;
	return 0.0f;
}

void Event::update()
{
	//delta time
	float currentTime = static_cast<float>(glfwGetTime());
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	//keys
	keysCheckFrame.clear();

	//check window resize
	windowResized = false;
	if (frameBufferResized)
	{
		frameBufferResized = false;
		windowResized = true;
	}
}

void Event::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	frameBufferResized = true;
	Window::windowWidth = width;
	Window::windowHeight = height;
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

	mod = mods;
}
