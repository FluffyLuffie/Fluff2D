#pragma once

#include "Deformer.h"

class RotationDeformer : public Deformer
{
	void deformEffect(glm::mat4& transform) override;
};

