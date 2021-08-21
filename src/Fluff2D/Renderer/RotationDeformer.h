#pragma once

#include <vector>

#include "ModelPart.h"

class RotationDeformer : public ModelPart
{
public:
	RotationDeformer(const std::string& partName);
	~RotationDeformer();

	void update() override;
	void render() override;
	void renderInspector() override;
};

