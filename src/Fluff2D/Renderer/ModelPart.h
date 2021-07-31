#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "../Core/Parameter.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Core/Log.h"

class Deformer;

class ModelPart
{
public:
	std::string name = "";
	glm::vec2 pos = glm::vec2(0.0f);
	glm::mat4 transform = glm::mat4(1.0f);

	float rotation = 0.0f;

	std::shared_ptr<Deformer> parent;
	std::vector<std::shared_ptr<ModelPart>> children;

	virtual void update();
	virtual void renderInspector() {}

	void setPos(int posX, int posY);
	void setPos(float posX, float posY);
};

