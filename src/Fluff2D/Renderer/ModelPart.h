#pragma once

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ModelPart
{
public:
	std::string name;
	glm::vec2 pos;
	float rotation = 0.0f;

	virtual void update();

	void setPos(int posX, int posY);
	void setPos(float posX, float posY);
};

