#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Vertex
{
public:
	glm::vec2 position;
	glm::vec2 texCoord;

	Vertex(glm::vec2 _pos, glm::vec2 _tex);

	Vertex(float posX, float posY, float texX, float texY);
};

