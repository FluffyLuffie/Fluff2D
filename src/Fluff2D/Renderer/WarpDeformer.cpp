#include "WarpDeformer.h"

WarpDeformer::WarpDeformer(const std::string& partName, int countX, int countY, float left, float bottom, float width, float height)
{
	type = ModelPart::PartType::warpDeformer;
	name = partName;
	boxCountX = countX;
	boxCountY = countY;

	pos = glm::vec2(left + width / 2, bottom + height / 2);

	//increase bounding box
	width *= borderBuffer;
	height *= borderBuffer;

	//create vertices for boxes
	vertices.reserve((boxCountX + 1) * (boxCountY + 1));
	originalVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
	prewarpedVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
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

	transform = parent->transform * localTransform;

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->update();
		for (int j = 0; j < children[i]->vertices.size(); j++)
		{
			glm::vec2 vertexLocalPos = children[i]->localTransform * glm::vec4(children[i]->prewarpedVertexPositions[j].x, children[i]->prewarpedVertexPositions[j].y, 0.0f, 1.0f);
			int boxX = static_cast<int>((vertexLocalPos.x - originalVertexPositions[0].x) / (originalVertexPositions[originalVertexPositions.size() - 1].x - originalVertexPositions[0].x) * boxCountX);
			int boxY = static_cast<int>((vertexLocalPos.y - originalVertexPositions[0].y) / (originalVertexPositions[originalVertexPositions.size() - 1].y - originalVertexPositions[0].y) * boxCountY);

			boxX = std::clamp(boxX, 0, boxCountX - 1);
			boxY = std::clamp(boxY, 0, boxCountY - 1);

			//get the 4 points of the quad the point is in
			int p1 = boxY * (boxCountX + 1) + boxX;
			int p2 = boxY * (boxCountX + 1) + boxX + 1;
			int p3 = (boxY + 1) * (boxCountX + 1) + boxX + 1;
			int p4 = (boxY + 1) * (boxCountX + 1) + boxX;

			float xRatio = (vertexLocalPos.x - originalVertexPositions[p1].x) / (originalVertexPositions[p2].x - originalVertexPositions[p1].x);
			float yRatio = (vertexLocalPos.y - originalVertexPositions[p1].y) / (originalVertexPositions[p4].y - originalVertexPositions[p1].y);

			float initialX = vertices[p1].position.x + xRatio * (vertices[p2].position.x - vertices[p1].position.x);
			float initialY = vertices[p1].position.y + yRatio * (vertices[p4].position.y - vertices[p1].position.y);

			children[i]->vertices[j].position = glm::inverse(children[i]->localTransform) * glm::vec4(initialX + (vertices[p4].position.x + xRatio * (vertices[p3].position.x - vertices[p4].position.x) - initialX) * yRatio, initialY + (vertices[p2].position.y + yRatio * (vertices[p3].position.y - vertices[p2].position.y) - initialY) * xRatio, 0.0f, 1.0f);
		}
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

	ImGui::DragFloat2("Position", (float*)&pos);
	ImGui::DragFloat("Rotation", &rotation);

	ImGui::Separator();
	ImGui::Text("Box count (x, y): %d, %d", boxCountX, boxCountY);
}
