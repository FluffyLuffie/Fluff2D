#include "Object.h"

void Object::setPos(int posX, int posY)
{
	pos.x = (float)posX;
	pos.y = (float)posY;
}

void Object::setPos(float posX, float posY)
{
	pos.x = posX;
	pos.y = posY;
}