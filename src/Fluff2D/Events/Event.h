#pragma once

#include <iostream>
#include <map>

#include "../Core/Window.h"

class Event
{
public:
	inline static GLFWwindow* GLFWWin;
	inline static float deltaTime = 0.0f;
	inline static ImGuiIO *io;

	inline static int mod = 0;

	inline static bool windowResized = false;
	inline static bool anyAction = true;
	inline static bool isFocused = false;
	inline static bool isHovered = false;

	static bool keyDown(int key);
	static bool keyPressed(int key);
	static bool keyReleased(int key);
	static float getScroll();
	static void update();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void cursor_position_callback(GLFWwindow* window, double xPos, double yPos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
	static void key_callback(GLFWwindow* window, int key, int scanmode, int action, int mods);

private:
	inline static float scroll = 0.0f;
	inline static float lastTime = 0.0f;
	inline static std::map<int, bool> keysCheckFrame;
	inline static bool frameBufferResized = false;
};

