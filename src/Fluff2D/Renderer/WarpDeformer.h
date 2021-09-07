#pragma once

#include <vector>
#include <algorithm>

#include "ModelPart.h"

class WarpDeformer : public ModelPart
{
public:
	WarpDeformer(const std::string& partName, int countX, int countY, float left, float bottom, float width, float height);
	~WarpDeformer();

	void update() override;
	void secondUpdate() override;
	void render() override;
	void renderInspector() override;

private:
	int boxCountX = 1, boxCountY = 1;
	float warpWidth = 0.0f, warpHeight = 0.0f, boxWidth = 0.0f, boxHeight = 0.0f;

	const float borderBuffer = 1.08f;
};

