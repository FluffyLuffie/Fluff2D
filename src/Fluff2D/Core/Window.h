#pragma once

#include "Fluff2D/Events/Event.h"

class Window
{
public:
	Window();
	~Window();

	void update();

	GLFWwindow* getWindow();
private:
	GLFWwindow* m_window;
	void init();
};

