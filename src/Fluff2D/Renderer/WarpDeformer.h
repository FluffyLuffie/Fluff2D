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
	void render() override;
	void renderInspector() override;
private:
	int boxCountX = 1, boxCountY = 1;

	const float borderBuffer = 1.08f;
};

