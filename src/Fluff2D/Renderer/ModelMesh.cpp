#include "ModelMesh.h"

ModelMesh::ModelMesh()
{
	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));
}

ModelMesh::~ModelMesh()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void ModelMesh::update()
{
	localTransform = glm::mat4(1.0f);
	localTransform = glm::translate(localTransform, glm::vec3(pos.x, pos.y, 0.0f));
	localTransform = glm::rotate(localTransform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	localTransform = glm::scale(localTransform, glm::vec3(scale, 1.0f));

	//assume parent always exists
	transform = parent->transform * localTransform;

	//testing stuff
	if (Event::keyDown(GLFW_KEY_RIGHT))
		localVertexPositions[0].x += 4;
	if (Event::keyDown(GLFW_KEY_LEFT))
		localVertexPositions[0].x -= 4;
	if (Event::keyDown(GLFW_KEY_UP))
		localVertexPositions[0].y += 4;
	if (Event::keyDown(GLFW_KEY_DOWN))
		localVertexPositions[0].y -= 4;
	if (Event::keyDown(GLFW_KEY_R))
		localVertexPositions[0] = glm::vec2(0.0f);

	if (parent->type == ModelPart::PartType::warpDeformer)
		for (int i = 0; i < localVertexPositions.size(); i++)
			prewarpedVertexPositions[i] = localTransform * glm::vec4(localVertexPositions[i], 0.0f, 1.0f);
}

void ModelMesh::secondUpdate()
{
	if (parent->type != ModelPart::PartType::warpDeformer)
		for (int i = 0; i < localVertexPositions.size(); i++)
			vertices[i].position = transform * glm::vec4(localVertexPositions[i], 0.0f, 1.0f);
}

void ModelMesh::render()
{
	updateVertexData();

	if (visible)
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void ModelMesh::renderInspector()
{
	ImGui::Text(name.c_str());

	ImGui::DragInt("Render Order", &renderOrder);

	ImGui::Text("Masking: %d", maskedMeshes.size());
	ImGui::Text("Masked: %d", maskedCount);
	ImGui::Separator();

	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();

	ImGui::DragFloat2("Position", (float*)&pos);
	ImGui::DragFloat("Rotation", &rotation);
	ImGui::DragFloat2("Scale", &scale.x, 0.01f);
	ImGui::Separator();
	ImGui::ColorEdit4("Mesh Color", &color[0]);
	ImGui::Separator();

	//maybe move this somewhere else
	if (ImGui::Button("Testing mesh generation"))
		ImGui::OpenPopup("Auto Mesh Generator");

	if (ImGui::BeginPopupModal("Auto Mesh Generator", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Not done yet, come back later");
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void ModelMesh::createBasicMesh(int layerX, int layerY, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight)
{
	atlasPositionX = layerX;
	atlasPositionY = layerY;
	textureWidth = layerW;
	textureHeight = layerH;
	flipped = flip;

	clearMeshData();

	vertices.reserve(5);
	localVertexPositions.reserve(5);
	originalVertexPositions.reserve(5);
	prewarpedVertexPositions.reserve(5);

	//create default vertices
	//order is center, top left, top right, bottom right, bottom left (basically clockwise)
	if (flip)
	{
		addVertex(0.0f, 0.0f, (layerX + layerW / 2.0f) / atlasWidth, (atlasHeight - layerY - layerH / 2.0f) / atlasHeight);
		addVertex(layerH / -2.0f, layerW / 2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight);
		addVertex(layerH / 2.0f, layerW / 2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight);
		addVertex(layerH / 2.0f, layerW / -2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight);
		addVertex(layerH / -2.0f, layerW / -2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight);
	}
	else
	{
		addVertex(0.0f, 0.0f, (layerX + layerW / 2.0f) / atlasWidth, (atlasHeight - layerY - layerH / 2.0f) / atlasHeight);
		addVertex(layerW / -2.0f, layerH / 2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight);
		addVertex(layerW / 2.0f, layerH / 2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight);
		addVertex(layerW / 2.0f, layerH / -2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight);
		addVertex(layerW / -2.0f, layerH / -2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight);
	}

	//make indices
	indices.reserve(12);
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(1);

	updateVertexData();
}

void ModelMesh::createBoxMesh(int boxCountX, int boxCountY, int atlasWidth, int atlasHeight)
{
	clearMeshData();

	//create vertices for boxes
	vertices.reserve((boxCountX + 1) * (boxCountY + 1));
	localVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
	originalVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
	prewarpedVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1));
	for (int y = 0; y <= boxCountY; y++)
	{
		for (int x = 0; x <= boxCountX; x++)
		{
			//add flipped method later
			if (flipped)
				addVertex(textureWidth * (-0.5f + 1.0f / boxCountX * x), textureHeight * (-0.5f + 1.0f / boxCountY * y), (atlasPositionX + ((float)textureWidth) * x / boxCountX) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight + ((float)textureHeight) * y / boxCountY) / atlasHeight);
			else
				addVertex(textureWidth * (-0.5f + 1.0f / boxCountX * x), textureHeight * (-0.5f + 1.0f / boxCountY * y), (atlasPositionX + ((float)textureWidth) * x / boxCountX) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight + ((float)textureHeight) * y / boxCountY) / atlasHeight);
		}
	}

	//indices
	indices.resize(boxCountX * boxCountY * 6);
	for (int vert = 0, tris = 0, y = 0; y < boxCountY; y++)
	{
		for (int x = 0; x < boxCountX; x++)
		{
			indices[tris] = vert;
			indices[tris + 1] = vert + boxCountX + 1;
			indices[tris + 2] = vert + 1;
			indices[tris + 3] = vert + 1;
			indices[tris + 4] = vert + boxCountX + 1;
			indices[tris + 5] = vert + boxCountX + 2;

			vert++;
			tris += 6;
		}
		vert++;
	}
	updateVertexData();
}
