#pragma once

#include "../Events/Event.h"

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
	GLFWwindow* m_window;
};

