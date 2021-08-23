#pragma once

#include "Settings.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image/stb_image.h"
#include "glm/vec3.hpp"

#include "Log.h"

class Window
{
public:
	Window();
	~Window();

	void init();

	void update();

	GLFWwindow* getWindow();

	inline static int windowWidth, windowHeight;
private:
	GLFWwindow* m_window = nullptr;
};

