#include "RotationDeformer.h"

RotationDeformer::RotationDeformer(const std::string& partName, float centerX, float centerY)
{
	type = ModelPart::PartType::rotationDeformer;
	name = partName;
	pos = glm::vec2(centerX, centerY);
	originalPos = glm::vec2(centerX, centerY);

	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

	//might need to make size not affected by other stuff
	addVertex(0.0f, 0.0f);
	addVertex(0.0f, 50.0f);

	originalVertexPositions[0] = glm::vec2(centerX, centerY);

	indices.push_back(0);
	indices.push_back(1);
}

RotationDeformer::~RotationDeformer()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void RotationDeformer::modelUpdate(std::unordered_map<std::string, float>& paramValues)
{
	localVertexPositions[0] = glm::vec2();
	vertices[0].position = transform * glm::vec4(localVertexPositions[0], 0.0f, 1.0f);

	localVertexPositions[1] = glm::vec2(0.0f, 20.0f);
	vertices[1].position = transform * glm::vec4(localVertexPositions[1], 0.0f, 1.0f);

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->updateTransform(paramValues);
		if (children[i]->type != ModelPart::PartType::mesh)
			children[i]->modelUpdate(paramValues);
	}
}

void RotationDeformer::render()
{
	updateVertexData();
	glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void RotationDeformer::renderInspector()
{
	ImGui::Text(name.c_str());

	ImGui::DragFloat2("Position", (float*)&pos);
	ImGui::DragFloat("Rotation", &rotation);
	ImGui::DragFloat2("Scale", &scale.x, 0.01f);
}
