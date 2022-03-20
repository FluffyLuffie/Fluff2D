#pragma once

#include <vector>

#include "ModelPart.h"

class RotationDeformer : public ModelPart
{
public:
	RotationDeformer(const std::string& partName, float centerX, float centerY);
	~RotationDeformer();

	void modelUpdate(const std::unordered_map<std::string, float>& paramValues) override;
	void render() override;
	void renderInspector() override;
};

