#pragma once

#include "Deformer.h"

class WarpDeformer : public Deformer
{
	void deformEffect(glm::mat4& transform) override;
};

