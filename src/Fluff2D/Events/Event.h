#pragma once

#include <iostream>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Core/Log.h"

class Event
{
public:
	inline static GLFWwindow* GLFWWin;
	inline static float deltaTime = 0.0f;

	static bool keyDown(int key);
	static bool keyPressed(int key);
	static bool keyReleased(int key);
	static float getScroll();
	static void calculateDeltaTime();
	static void resetKeys();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xPos, double yPos);
	static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
	static void key_callback(GLFWwindow* window, int key, int scanmode, int action, int mods);

private:
	inline static float scroll = 0.0f;
	inline static float lastTime = 0.0f;
	inline static std::map<int, bool> keysCheckFrame;
};

