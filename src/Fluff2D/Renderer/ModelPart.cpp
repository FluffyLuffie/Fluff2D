#include "ModelPart.h"

void ModelPart::updateTransform()
{
	localTransform = glm::mat4(1.0f);
	localTransform = glm::translate(localTransform, glm::vec3(pos.x, pos.y, 0.0f));
	localTransform = glm::rotate(localTransform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	localTransform = glm::scale(localTransform, glm::vec3(scale, 1.0f));

	//assume parent always exists
	transform = parent->transform * localTransform;
}

void ModelPart::addVertex(float xCoord, float yCoord)
{
	vertices.emplace_back(xCoord, yCoord);
	localVertexPositions.emplace_back(xCoord, yCoord);
	originalVertexPositions.emplace_back(xCoord, yCoord);
	deltaVertexPositions.emplace_back(0.0f, 0.0f);
}

void ModelPart::addVertex(float xCoord, float yCoord, float xTexCoord, float yTexCoord)
{
	vertices.emplace_back(xCoord, yCoord, xTexCoord, yTexCoord);
	localVertexPositions.emplace_back(xCoord, yCoord);
	originalVertexPositions.emplace_back(xCoord, yCoord);
	deltaVertexPositions.emplace_back(0.0f, 0.0f);
}

void ModelPart::clearMeshData()
{
	vertices.clear();
	localVertexPositions.clear();
	originalVertexPositions.clear();
	deltaVertexPositions.clear();
	indices.clear();
}

void ModelPart::updateVertexData()
{
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

	//set vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	//set tex coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);
}
