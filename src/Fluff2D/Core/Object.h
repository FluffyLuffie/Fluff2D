#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "../Core/Window.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Object
{
public:
	std::string name = "";

	glm::vec2 pos = glm::vec2(0.0f);
	float rotation = 0.0f;

	glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 localTransform = glm::mat4(1.0f);

	virtual void update() {}
	virtual void render() {}

	void setPos(int posX, int posY);
	void setPos(float posX, float posY);
};

