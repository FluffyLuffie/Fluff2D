#pragma once

#include <vector>
#include <algorithm>

#include "ModelPart.h"

class WarpDeformer : public ModelPart
{
public:
	WarpDeformer(const std::string& partName, int countX, int countY, float centerX, float centerY, float width, float height);
	~WarpDeformer();

	void update() override;
	void render() override;
	void renderInspector() override;

	glm::vec2 unwarpPoint(glm::vec2 point);

private:
	int boxCountX = 1, boxCountY = 1;
	float warpWidth = 0.0f, warpHeight = 0.0f, boxWidth = 0.0f, boxHeight = 0.0f;

	const float borderBuffer = 1.08f;

	glm::vec2 warpPoint(glm::vec2 point);

	//also changes value of point to be within box
	int findBox(glm::vec2 &point);

	//from https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	float minDistance(glm::vec2 p1, glm::vec2 p2, glm::vec2 distanceTo);

	//from https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
	bool onSegment(glm::vec2 p, glm::vec2 q, glm::vec2 r);
	int orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r);
	bool doIntersect(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2);
};

