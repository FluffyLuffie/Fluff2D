#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Event
{
public:
	inline static GLFWwindow* GLFWWin;
	inline static float deltaTime = 0.0f;

	static bool keyPressed(int key);
	static float getScroll();
	static void calculateDeltaTime();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void processInput(GLFWwindow* window);
	static void mouse_callback(GLFWwindow* window, double xPos, double yPos);
	static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

private:
	inline static float scroll = 0.0f;
	inline static float lastTime = 0.0f;
};

