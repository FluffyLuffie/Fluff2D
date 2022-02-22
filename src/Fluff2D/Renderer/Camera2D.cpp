#include "Camera2D.h"

void Camera2D::update()
{
	float scroll = Event::getScroll();
	if (scroll != 0.0f)
	{
		if (scroll < 0.0f && scale > 0.01f)
			scale *= 0.9f;
		else if (scroll > 0.0f && scale < 50)
			scale *= 1.1f;
	}

	if (Event::keyDown(GLFW_KEY_W))
		pos.y += Event::deltaTime * 500 / scale;
	if (Event::keyDown(GLFW_KEY_A))
		pos.x -= Event::deltaTime * 500 / scale;
	if (Event::keyDown(GLFW_KEY_S))
		pos.y -= Event::deltaTime * 500 / scale;
	if (Event::keyDown(GLFW_KEY_D))
		pos.x += Event::deltaTime * 500 / scale;

	//testing rotation, might just delete
	/*
	if (Event::keyDown(GLFW_KEY_Q))
		rotation += 100 * Event::deltaTime;
	if (Event::keyDown(GLFW_KEY_E))
		rotation -= 100 * Event::deltaTime;
	*/

	projection = glm::ortho(Window::width / -2.0f / scale, Window::width / 2.0f / scale, Window::height / -2.0f / scale, Window::height / 2.0f / scale, -1.0f, 1.0f);
	projection = glm::translate(projection, glm::vec3(-pos.x, -pos.y, 0.0f));
	//projection = glm::rotate(projection, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
}
