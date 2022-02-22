#include "WarpDeformer.h"

WarpDeformer::WarpDeformer(const std::string& partName, int countX, int countY, float centerX, float centerY, float width, float height)
{
	type = ModelPart::PartType::warpDeformer;
	name = partName;
	boxCountX = countX;
	boxCountY = countY;

	pos = glm::vec2(centerX, centerY);

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
}

WarpDeformer::~WarpDeformer()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void WarpDeformer::update()
{
	localTransform = glm::mat4(1.0f);
	localTransform = glm::translate(localTransform, glm::vec3(pos.x, pos.y, 0.0f));
	localTransform = glm::rotate(localTransform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	localTransform = glm::scale(localTransform, glm::vec3(scale, 1.0f));

	transform = parent->transform * localTransform;

	//maybe move warp deformation here, where the child calls it before updating their child
	if (parent->type == ModelPart::PartType::warpDeformer)
	{
		for (int i = 0; i < localVertexPositions.size(); i++)
			vertices[i].position = localTransform * glm::vec4(localVertexPositions[i], 0.0f, 1.0f);
		std::dynamic_pointer_cast<WarpDeformer>(parent)->applyWarp(this);
	}
	else
		for (int i = 0; i < localVertexPositions.size(); i++)
			vertices[i].position = transform * glm::vec4(localVertexPositions[i], 0.0f, 1.0f);

	for (int i = 0; i < children.size(); i++)
		children[i]->update();
}

void WarpDeformer::render()
{
	updateVertexData();

	glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void WarpDeformer::renderInspector()
{
	ImGui::Text(name.c_str());

	ImGui::DragFloat2("Position", (float*)&pos);
	ImGui::DragFloat("Rotation", &rotation);
	ImGui::DragFloat2("Scale", &scale.x, 0.01f);

	ImGui::Separator();
	ImGui::Text("Box count (x, y): %d, %d", boxCountX, boxCountY);
}

void WarpDeformer::applyWarp(ModelPart *part)
{
	for (int i = 0; i < part->vertices.size(); i++)
	{
		int boxX = static_cast<int>((part->vertices[i].position.x - originalVertexPositions[0].x) / warpWidth * boxCountX);
		int boxY = static_cast<int>((part->vertices[i].position.y - originalVertexPositions[0].y) / warpHeight * boxCountY);

		boxX = std::clamp(boxX, 0, boxCountX - 1);
		boxY = std::clamp(boxY, 0, boxCountY - 1);

		//get the 4 points of the quad the point is in
		int p1 = boxY * (boxCountX + 1) + boxX;
		int p2 = boxY * (boxCountX + 1) + boxX + 1;
		int p3 = (boxY + 1) * (boxCountX + 1) + boxX + 1;
		int p4 = (boxY + 1) * (boxCountX + 1) + boxX;

		float xRatio = (part->vertices[i].position.x - originalVertexPositions[p1].x) / boxWidth;
		float yRatio = (part->vertices[i].position.y - originalVertexPositions[p1].y) / boxHeight;

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

		//set vertex position
		part->vertices[i].position = transform * glm::vec4(initialX + xOffset + (localVertexPositions[p4].x + xClamped * (localVertexPositions[p3].x - localVertexPositions[p4].x) - initialX) * yClamped, initialY + yOffset + (localVertexPositions[p2].y + yClamped * (localVertexPositions[p3].y - localVertexPositions[p2].y) - initialY) * xClamped, 0.0f, 1.0f);
	}
}
