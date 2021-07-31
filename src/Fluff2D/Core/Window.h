#pragma once

#include "../Events/Event.h"

#include "stb_image/stb_image.h"
#include "glm/vec3.hpp"

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

