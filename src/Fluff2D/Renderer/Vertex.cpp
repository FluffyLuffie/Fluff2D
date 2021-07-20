#include "Vertex.h"

Vertex::Vertex(glm::vec2 _pos, glm::vec2 _tex)
{
	position = _pos;
	texCoord = _tex;
}

Vertex::Vertex(float posX, float posY, float texX, float texY)
{
	position.x = posX;
	position.y = posY;
	texCoord.x = texX;
	texCoord.y = texY;
}

void Vertex::setVertex(float posX, float posY, float texX, float texY)
{
	position.x = posX;
	position.y = posY;
	texCoord.x = texX;
	texCoord.y = texY;
}
