#include "RotationDeformer.h"

RotationDeformer::RotationDeformer(const std::string& partName, float centerX, float centerY)
{
	type = ModelPart::PartType::rotationDeformer;
	name = partName;
	pos = glm::vec2(centerX, centerY);
	originalPos = glm::vec2(centerX, centerY);
	basePos = glm::vec2(centerX, centerY);

	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);

	//might need to make size not affected by other stuff
	addVertex(0.0f, 0.0f);
	addVertex(0.0f, 50.0f);

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
	vertices[0].position = transform * glm::vec4(localVertexPositions[0], 0.0f, 1.0f);
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

	bool dataChanged = false;

	dataChanged |= ImGui::DragFloat2("Position", (float*)&pos);
	dataChanged |= ImGui::DragFloat("Rotation", &rotation);
	dataChanged |= ImGui::DragFloat2("Scale", &scale.x, 0.01f);

	if (dataChanged)
	{
		if (keyformIndex != -1)
		{
			keyforms[keyformIndex].position = pos - basePos;
			keyforms[keyformIndex].rotation = rotation - baseRotation;
			keyforms[keyformIndex].scale = scale - baseScale;
		}
		else
		{
			basePos = pos;
			baseRotation = rotation;
			baseScale = scale;
		}
	}
}

void RotationDeformer::changeCenterPoint(glm::vec2 offset)
{
	for (int i = 0; i < children.size(); i++)
	{
		glm::mat4 temp = glm::mat4(1.0f);
		temp = glm::rotate(temp, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		temp = glm::scale(temp, glm::vec3(scale, 1.0f));

		glm::vec2 childOffset = glm::inverse(temp) * glm::vec4(offset, 0.0f, 1.0f);
		children[i]->basePos -= childOffset;
	}
}
