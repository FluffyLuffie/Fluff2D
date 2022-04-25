#include "WarpDeformer.h"

WarpDeformer::WarpDeformer(const std::string& partName, int countX, int countY, float centerX, float centerY, float width, float height)
{
	type = ModelPart::PartType::warpDeformer;
	name = partName;
	boxCountX = countX;
	boxCountY = countY;

	pos = glm::vec2(centerX, centerY);
	originalPos = glm::vec2(centerX, centerY);
	basePos = glm::vec2(centerX, centerY);

	//increase bounding box
	width *= borderBuffer;
	height *= borderBuffer;

	warpWidth = width;
	warpHeight = height;
	boxWidth = width / countX;
	boxHeight = height / countY;

	//create vertices for boxes
	vertices.reserve((boxCountX + 1) * (boxCountY + 1));
	localVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
	originalVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
	for (int y = 0; y <= boxCountY; y++)
	{
		for (int x = 0; x <= boxCountX; x++)
		{
			addVertex(width * (-0.5f + 1.0f / boxCountX * x), height * (-0.5f + 1.0f / boxCountY * y));
		}
	}

	//create indices
	indices.reserve((boxCountX * (boxCountY + 1) + (boxCountX + 1) * boxCountY) * 2);
	//horizontal
	for (int y = 0; y <= boxCountY; y++)
	{
		for (int x = 0; x < boxCountX; x++)
		{
			indices.push_back(y * (boxCountX + 1) + x);
			indices.push_back(y * (boxCountX + 1) + x + 1);
		}
	}
	//vertical
	for (int y = 0; y < boxCountY; y++)
	{
		for (int x = 0; x <= boxCountX; x++)
		{
			indices.push_back(y * (boxCountX + 1) + x);
			indices.push_back((y + 1) * (boxCountX + 1) + x);
		}
	}

	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
}

WarpDeformer::~WarpDeformer()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void WarpDeformer::modelUpdate(std::unordered_map<std::string, float>& paramValues)
{
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->updateTransform(paramValues);

		if (children[i]->type == ModelPart::PartType::rotationDeformer)
		{
			//change position
			glm::mat4 temp = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(children[i]->pos, 0.0f)), glm::vec3(children[i]->scale, 1.0f));
			children[i]->warpTransform(warpPoint(children[i]->localTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
		}
		else
		{
			for (int j = 0; j < children[i]->vertices.size(); j++)
			{
				//set vertex position
				children[i]->vertices[j].position = transform * glm::vec4(warpPoint(children[i]->localTransform * glm::vec4(children[i]->localVertexPositions[j], 0.0f, 1.0f)), 0.0f, 1.0f);
			}
		}

		if (children[i]->type != ModelPart::PartType::mesh)
			children[i]->modelUpdate(paramValues);
	}
}

void WarpDeformer::render()
{
	updateVertexData();

	glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void WarpDeformer::renderInspector()
{
	ImGui::Text(name.c_str());

	bool dataChanged = false;

	dataChanged |= ImGui::DragFloat2("Position", (float*)&pos);
	dataChanged |= ImGui::DragFloat("Rotation", &rotation);
	dataChanged |= ImGui::DragFloat2("Scale", &scale.x, 0.01f);

	if (dataChanged)
	{
		if (keyformIndex != -1)
		{
			keyforms[keyformIndex].deltaPosition = pos - basePos;
			keyforms[keyformIndex].deltaRotation = rotation - baseRotation;
			keyforms[keyformIndex].deltaScale = scale - baseScale;
		}
		else if (!keyforms.size())
		{
			basePos = pos;
			baseRotation = rotation;
			baseScale = scale;
		}
	}

	ImGui::Separator();
	ImGui::Text("Box count (x, y): %d, %d", boxCountX, boxCountY);
}

//unwarps a point in local coordinates
//concave corners a bit weird, fix later
glm::vec2 WarpDeformer::unwarpPoint(glm::vec2 point)
{
	//find which box the point is in
	glm::vec2 originalPoint = point;
	int boxNum = findBox(point);
	//std::cout << "box: " << boxNum << std::endl;

	int boxX = boxNum % boxCountX;
	int boxY = boxNum / boxCountX;

	glm::vec2 p1 = localVertexPositions[boxX + boxY * (boxCountX + 1)];
	glm::vec2 p2 = localVertexPositions[boxX + 1 + boxY * (boxCountX + 1)];
	glm::vec2 p3 = localVertexPositions[boxX + (boxY + 1) * (boxCountX + 1)];
	glm::vec2 p4 = localVertexPositions[boxX + 1 + (boxY + 1) * (boxCountX + 1)];

	float aX = (p4.x - p2.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p4.y - p2.y);
	float aY = (p4.x - p3.x) * (p2.y - p1.y) - (p2.x - p1.x) * (p4.y - p3.y);
	//if in uneditted box
	if (aX == 0.0f || aY == 0.0f)
		return point + originalPoint - point;

	float bX = p2.x * (p3.y - p1.y) + p1.y * (p4.x - p2.x) - point.y * (p4.x - p2.x) + point.y * (p3.x - p1.x) - p1.x * (p4.y - p2.y) - p2.y * (p3.x - p1.x) + point.x * (p4.y - p2.y) - point.x * (p3.y - p1.y);
	float cX = p2.x * p1.y - p2.x * point.y + p1.x * point.y - p1.x * p2.y + p2.y * point.x - p1.y * point.x;

	//if in editted box
	float bY = p3.x * (p2.y - p1.y) + p1.y * (p4.x - p3.x) - point.y * (p4.x - p3.x) + point.y * (p2.x - p1.x) - p1.x * (p4.y - p3.y) - p3.y * (p2.x - p1.x) + point.x * (p4.y - p3.y) - point.x * (p2.y - p1.y);
	float cY = p3.x * p1.y - p3.x * point.y + p1.x * point.y - p1.x * p3.y + p3.y * point.x - p1.y * point.x;

	float rYMax = (-bX + std::sqrt(bX * bX - 4 * aX * cX)) / (2 * aX);
	float rXMin = (-bY - std::sqrt(bY * bY - 4 * aY * cY)) / (2 * aY);

	//bottom left corner + offset
	return originalVertexPositions[boxX + boxY * (boxCountX + 1)] + glm::vec2(boxWidth * rXMin, boxHeight * rYMax) + originalPoint - point;

	//don't need the other ones?
	/*
	float rYMin = (-bX - std::sqrt(bX * bX - 4 * aX * cX)) / (2 * aX);
	float rXMax = (-bY + std::sqrt(bY * bY - 4 * aY * cY)) / (2 * aY);
	std::cout << "a: " << aX << " b: " << bX << "  c: " << cX << std::endl;
	std::cout << "quad: " << bX * bX - 4 * aX * cX << std::endl;
	std::cout << "rYMax: " << rYMax << " ryMin: " << rYMin << std::endl;
	std::cout << "rXMax: " << rXMax << " rXMin: " << rXMin << std::endl;
	*/
}

glm::vec2 WarpDeformer::warpPoint(glm::vec2 point)
{
	int boxX = static_cast<int>((point.x - originalVertexPositions[0].x) / warpWidth * boxCountX);
	int boxY = static_cast<int>((point.y - originalVertexPositions[0].y) / warpHeight * boxCountY);

	boxX = std::clamp(boxX, 0, boxCountX - 1);
	boxY = std::clamp(boxY, 0, boxCountY - 1);

	//get the 4 points of the quad the point is in
	int p1 = boxY * (boxCountX + 1) + boxX;
	int p2 = boxY * (boxCountX + 1) + boxX + 1;
	int p3 = (boxY + 1) * (boxCountX + 1) + boxX + 1;
	int p4 = (boxY + 1) * (boxCountX + 1) + boxX;

	float xRatio = (point.x - originalVertexPositions[p1].x) / boxWidth;
	float yRatio = (point.y - originalVertexPositions[p1].y) / boxHeight;

	float xClamped = std::clamp(xRatio, 0.0f, 1.0f);
	float yClamped = std::clamp(yRatio, 0.0f, 1.0f);

	//offsets are reversed for some reason idk how matrices work
	float xOffset = 0.0f, yOffset = 0.0f;
	if (xRatio > 1.0f)
		xOffset = (xRatio - 1.0f) * boxWidth;
	else if (xRatio < 0.0f)
		xOffset = xRatio * boxWidth;
	if (yRatio > 1.0f)
		yOffset = (yRatio - 1.0f) * boxHeight;
	else if (yRatio < 0.0f)
		yOffset = yRatio * boxHeight;

	float initialX = localVertexPositions[p1].x + xClamped * (localVertexPositions[p2].x - localVertexPositions[p1].x);
	float initialY = localVertexPositions[p1].y + yClamped * (localVertexPositions[p4].y - localVertexPositions[p1].y);

	return glm::vec2(initialX + xOffset + (localVertexPositions[p4].x + xClamped * (localVertexPositions[p3].x - localVertexPositions[p4].x) - initialX) * yClamped, initialY + yOffset + (localVertexPositions[p2].y + yClamped * (localVertexPositions[p3].y - localVertexPositions[p2].y) - initialY) * xClamped);
}

int WarpDeformer::findBox(glm::vec2 &point)
{
	//if point is in box
	for (int y = 0; y < boxCountY; y++)
	{
		for (int x = 0; x < boxCountX; x++)
		{
			int crossCount = 0;

			//bottom left, bottom right, top left, top right
			int p1 = x + y * (boxCountX + 1);
			int p2 = x + 1 + y * (boxCountX + 1);
			int p3 = x + (y + 1) * (boxCountX + 1);
			int p4 = x + 1 + (y + 1) * (boxCountX + 1);

			//make a line segment from point straight up, until top
			float top = localVertexPositions[p1].y;
			if (top < localVertexPositions[p2].y)
				top = localVertexPositions[p2].y;
			if (top < localVertexPositions[p3].y)
				top = localVertexPositions[p3].y;
			if (top < localVertexPositions[p4].y)
				top = localVertexPositions[p4].y;
			top++;

			if (doIntersect(localVertexPositions[p1], localVertexPositions[p2], point, glm::vec2(point.x, top)))
				crossCount++;
			if (doIntersect(localVertexPositions[p2], localVertexPositions[p4], point, glm::vec2(point.x, top)))
				crossCount++;
			if (doIntersect(localVertexPositions[p4], localVertexPositions[p3], point, glm::vec2(point.x, top)))
				crossCount++;
			if (doIntersect(localVertexPositions[p3], localVertexPositions[p1], point, glm::vec2(point.x, top)))
				crossCount++;

			if (crossCount % 2 == 1)
				return x + y * boxCountX;
		}
	}

	//if outside boxes
	//check if below
	for (int x = 0; x < boxCountX; x++)
	{
		glm::vec2 p1 = localVertexPositions[x];
		glm::vec2 p2 = localVertexPositions[x + 1];
		float top = std::max(p1.y, p2.y) + 1.0f;

		if (doIntersect(p1, p2, point, glm::vec2(point.x, top)))
		{
			point.y = (p2.y - p1.y) / (p2.x - p1.x) * point.x + (p1.y - (p2.y - p1.y) / (p2.x - p1.x) * p1.x);
			return x;
		}
	}
	//check if above
	for (int x = 0; x < boxCountX; x++)
	{
		glm::vec2 p1 = localVertexPositions[x + boxCountY * (boxCountX + 1)];
		glm::vec2 p2 = localVertexPositions[x + 1 + boxCountY * (boxCountX + 1)];
		float bottom = std::min(p1.y, p2.y) - 1.0f;

		if (doIntersect(p1, p2, point, glm::vec2(point.x, bottom)))
		{
			point.y = (p2.y - p1.y) / (p2.x - p1.x) * point.x + (p1.y - (p2.y - p1.y) / (p2.x - p1.x) * p1.x);
			return x + (boxCountY - 1) * boxCountX;
		}
	}
	//check if left
	for (int y = 0; y < boxCountY; y++)
	{
		glm::vec2 p1 = localVertexPositions[y * (boxCountX + 1)];
		glm::vec2 p2 = localVertexPositions[(y + 1) * (boxCountX + 1)];
		float right = std::max(p1.x, p2.x) + 1.0f;

		if (doIntersect(p1, p2, point, glm::vec2(right, point.y)))
		{
			point.x = (p2.x - p1.x) / (p2.y - p1.y) * point.y + (p1.x - (p2.x - p1.x) / (p2.y - p1.y) * p1.y);
			return y * boxCountX;
		}
	}
	//check if right
	for (int y = 0; y < boxCountY; y++)
	{
		glm::vec2 p1 = localVertexPositions[boxCountX + y * (boxCountX + 1)];
		glm::vec2 p2 = localVertexPositions[boxCountX + (y + 1) * (boxCountX + 1)];
		float left = std::min(p1.x, p2.x) - 1.0f;

		if (doIntersect(p1, p2, point, glm::vec2(left, point.y)))
		{
			point.x = (p2.x - p1.x) / (p2.y - p1.y) * point.y + (p1.x - (p2.x - p1.x) / (p2.y - p1.y) * p1.y);
			return boxCountX - 1 + y * boxCountX;
		}
	}

	//check if corners
	//check bottom left
	if (point.x <= localVertexPositions[1].x && point.y <= localVertexPositions[boxCountX + 1].y)
	{
		point = localVertexPositions[0];
		return 0;
	}
	//check bottom right
	if (point.x >= localVertexPositions[boxCountX - 1].x && point.y <= localVertexPositions[boxCountX * 2 + 1].y)
	{
		point = localVertexPositions[boxCountX];
		return boxCountX - 1;
	}
	//check top left
	if (point.x <= localVertexPositions[boxCountY * (boxCountX + 1) + 1].x && point.y >= localVertexPositions[(boxCountY - 1) * (boxCountX + 1)].y)
	{
		point = localVertexPositions[boxCountY * (boxCountX + 1)];
		return (boxCountY - 1) * boxCountX;
	}

	//don't really need to check?
	point = localVertexPositions[boxCountX + boxCountY * (boxCountX + 1)];
	return boxCountX * boxCountY - 1;

	//check if top right
	//if (point.x >= localVertexPositions[boxCountX - 1 + boxCountY * (boxCountX + 1)].x && point.y >= localVertexPositions[boxCountX + (boxCountY - 1) * (boxCountX + 1)].y)
	//	return boxCountX * boxCountY - 1;
}

float WarpDeformer::minDistance(glm::vec2 p1, glm::vec2 p2, glm::vec2 distanceTo)
{
	return std::abs((p2.x - p1.x) * (p1.y - distanceTo.y) - (p1.x - distanceTo.x) * (p2.y - p1.y)) / glm::distance(p1, p2);

	/*
	float lineLength = glm::distance(p1, p2);
	return std::min(std::abs((p2.x - p1.x) / lineLength * (p1.y - distanceTo.y) - (p2.y - p1.y) / lineLength * (p1.x - distanceTo.x)),
					std::abs((p1.x - p2.x) / lineLength * (p2.y - distanceTo.y) - (p1.y - p2.y) / lineLength * (p2.x - distanceTo.x)));
	*/
}

bool WarpDeformer::onSegment(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
		q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
		return true;

	return false;
}

int WarpDeformer::orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	float val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	//magic number, hopefully it works
	if (std::abs(val) <= 0.000001f) return 0;

	return (val > 0) ? 1 : 2;
}

bool WarpDeformer::doIntersect(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2)
{
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	if (o1 != o2 && o3 != o4)
		return true;

	if (o1 == 0 && onSegment(p1, p2, q1)) return true;
	if (o2 == 0 && onSegment(p1, q2, q1)) return true;
	if (o3 == 0 && onSegment(p2, p1, q2)) return true;
	if (o4 == 0 && onSegment(p2, q1, q2)) return true;

	return false;
}
