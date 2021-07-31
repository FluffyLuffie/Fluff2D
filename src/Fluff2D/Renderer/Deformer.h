#pragma once

#include "ModelPart.h"
#include <vector>

class Deformer : public ModelPart
{
public:
	virtual void deformEffect(glm::mat4& transform) { std::cout << "Deformer::deformEffect() called. This does nothing" << std::endl; }
};

