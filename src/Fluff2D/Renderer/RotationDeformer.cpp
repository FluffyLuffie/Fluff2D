#include "RotationDeformer.h"

RotationDeformer::RotationDeformer(const std::string& partName)
{
	type = ModelPart::PartType::rotationDeformer;
	name = partName;

	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

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

void RotationDeformer::update()
{
	localTransform = glm::mat4(1.0f);
	localTransform = glm::translate(localTransform, glm::vec3(pos.x, pos.y, 0.0f));
	localTransform = glm::rotate(localTransform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	transform = parent->transform * localTransform;

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->update();
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
}
