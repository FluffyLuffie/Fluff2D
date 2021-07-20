#include "ModelPart.h"

void ModelPart::update()
{
	std::cout << "ModelPart update should not be called" << std::endl;
}

void ModelPart::setPos(int posX, int posY)
{
	pos.x = (float)posX;
	pos.y = (float)posY;
}

void ModelPart::setPos(float posX, float posY)
{
	pos.x = posX;
	pos.y = posY;
}
