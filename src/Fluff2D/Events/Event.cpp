#include "Event.h"

bool Event::keyDown(int key)
{
	if (!io->WantCaptureKeyboard && isFocused && glfwGetKey(GLFWWin, key) == GLFW_PRESS)
	{
		anyAction = true;
		return true;
	}
	return false;
}

bool Event::keyPressed(int key)
{
	if (!io->WantCaptureKeyboard && isFocused && keysCheckFrame.find(key) != keysCheckFrame.end())
	{
		anyAction = true;
		return keysCheckFrame[key];
	}
	return false;
}

bool Event::keyReleased(int key)
{
	if (!io->WantCaptureKeyboard && isFocused && keysCheckFrame.find(key) != keysCheckFrame.end())
	{
		anyAction = true;
		return !keysCheckFrame[key];
	}
	return false;
}

float Event::getScroll()
{
	if (isHovered)
		return scroll;
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

	scroll = 0.0f;

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
	anyAction = true;
	glViewport(0, 0, width, height);
	frameBufferResized = true;
	Window::width = width;
	Window::height = height;
}

void Event::cursor_position_callback(GLFWwindow* window, double xPos, double yPos)
{
	anyAction = true;
}

void Event::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	anyAction = true;
}

void Event::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	anyAction = true;
	scroll = (float)yOffset;
}

void Event::key_callback(GLFWwindow* window, int key, int scanmode, int action, int mods)
{
	anyAction = true;
	if (action == GLFW_PRESS || action == GLFW_RELEASE)
		keysCheckFrame[key] = action;

	mod = mods;
}
