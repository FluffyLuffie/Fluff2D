#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "../Events/Event.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Object
{
public:
	std::string name = "";

	glm::vec2 pos = glm::vec2(0.0f);
	float rotation = 0.0f;
	glm::vec2 scale = glm::vec2(1.0f);

	glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 localTransform = glm::mat4(1.0f);

	virtual void update() {}
};

