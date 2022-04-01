#include "ModelMesh.h"

ModelMesh::ModelMesh()
{
	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

ModelMesh::~ModelMesh()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void ModelMesh::render()
{
	updateVertexData();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void ModelMesh::renderInspector()
{
	ImGui::Text(name.c_str());
	bool dataChanged = false;

	dataChanged |= ImGui::DragInt("Render Order", &renderOrder);
	ImGui::Separator();

	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();

	dataChanged |= ImGui::DragFloat2("Position", (float*)&pos);
	dataChanged |= ImGui::DragFloat("Rotation", &rotation);
	dataChanged |= ImGui::DragFloat2("Scale", &scale.x, 0.01f);
	ImGui::Separator();
	dataChanged |= ImGui::ColorEdit4("Mesh Color", &color[0]);
	ImGui::Separator();

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

	//ImGui::Text("%s (%d, %d) (%d, %d)", flipped ? "true" : "false", textureWidth, textureHeight, atlasPositionX, atlasPositionY);

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

void ModelMesh::addMeshVertex(glm::vec2 vPos, int atlasWidth, int atlasHeight)
{
	if (flipped)
		addVertex(vPos.x, vPos.y, (atlasPositionX + textureWidth / 2.0f - vPos.y) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight / 2.0f + vPos.x) / atlasHeight);
	else
		addVertex(vPos.x, vPos.y, (atlasPositionX + textureWidth / 2.0f + vPos.x) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight / 2.0f + vPos.y) / atlasHeight);

	vertices[vertices.size() - 1].position = transform * glm::vec4(vertices[vertices.size() - 1].position, 0.0f, 1.0f);
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

	//create default vertices
	//order is center, top left, top right, bottom right, bottom left (basically clockwise)
	if (flip)
	{
		addMeshVertex(glm::vec2(), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / -2.0f, layerW / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / 2.0f, layerW / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / 2.0f, layerW / -2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / -2.0f, layerW / -2.0f), atlasWidth, atlasHeight);
		//addVertex(0.0f, 0.0f,						(layerX + layerW / 2.0f) / atlasWidth,		(atlasHeight - layerY - layerH / 2.0f) / atlasHeight);
		// 
		//addVertex(layerH / -2.0f, layerW / 2.0f,	(float)layerX / atlasWidth,					(float)(atlasHeight - layerY - layerH) / atlasHeight);
		//addVertex(layerH / 2.0f, layerW / 2.0f,	(float)layerX / atlasWidth,					(float)(atlasHeight - layerY) / atlasHeight);
		//addVertex(layerH / 2.0f, layerW / -2.0f,	(float)(layerX + layerW) / atlasWidth,		(float)(atlasHeight - layerY) / atlasHeight);
		//addVertex(layerH / -2.0f, layerW / -2.0f, (float)(layerX + layerW) / atlasWidth,		(float)(atlasHeight - layerY - layerH) / atlasHeight);
	}
	else
	{
		addMeshVertex(glm::vec2(), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / -2.0f, layerH / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / 2.0f, layerH / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / 2.0f, layerH / -2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / -2.0f, layerH / -2.0f), atlasWidth, atlasHeight);

		/*
		addVertex(0.0f, 0.0f,						(layerX + layerW / 2.0f) / atlasWidth,		(atlasHeight - layerY - layerH / 2.0f) / atlasHeight);

		addVertex(layerW / -2.0f, layerH / 2.0f,	(float)layerX / atlasWidth,					(float)(atlasHeight - layerY) / atlasHeight);
		addVertex(layerW / 2.0f, layerH / 2.0f,		(float)(layerX + layerW) / atlasWidth,		(float)(atlasHeight - layerY) / atlasHeight);
		addVertex(layerW / 2.0f, layerH / -2.0f,	(float)(layerX + layerW) / atlasWidth,		(float)(atlasHeight - layerY - layerH) / atlasHeight);
		addVertex(layerW / -2.0f, layerH / -2.0f,	(float)layerX / atlasWidth,					(float)(atlasHeight - layerY - layerH) / atlasHeight);
		*/
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
}

void ModelMesh::createTriMesh(int boxCountX, int boxCountY, int atlasWidth, int atlasHeight)
{
	clearMeshData();

	//create vertices for boxes
	vertices.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);
	localVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);
	originalVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);

	if (flipped)
	{
		//normal box
		for (int y = 0; y <= boxCountY; y++)
			for (int x = 0; x <= boxCountX; x++)
				addMeshVertex(glm::vec2(textureHeight * (-0.5f + 1.0f / boxCountX * x), textureWidth * (-0.5f + 1.0f / boxCountY * y)), atlasWidth, atlasHeight);

		//put dot in middle
		for (int y = 0; y < boxCountY; y++)
			for (int x = 0; x < boxCountX; x++)
				addMeshVertex(glm::vec2(textureHeight * (-0.5f + 1.0f / boxCountX * (x + 0.5f)), textureWidth * (-0.5f + 1.0f / boxCountY * (y + 0.5f))), atlasWidth, atlasHeight);
	}
	else
	{
		//normal box
		for (int y = 0; y <= boxCountY; y++)
			for (int x = 0; x <= boxCountX; x++)
				addMeshVertex(glm::vec2(textureWidth * (-0.5f + 1.0f / boxCountX * x), textureHeight * (-0.5f + 1.0f / boxCountY * y)), atlasWidth, atlasHeight);

		//put dot in middle
		for (int y = 0; y < boxCountY; y++)
			for (int x = 0; x < boxCountX; x++)
				addMeshVertex(glm::vec2(textureWidth * (-0.5f + 1.0f / boxCountX * (x + 0.5f)), textureHeight * (-0.5f + 1.0f / boxCountY * (y + 0.5f))), atlasWidth, atlasHeight);
	}

	Triangulator::triangulate(vertices, indices);
}

void ModelMesh::startMeshEdit()
{
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(originalPos, 0.0f));
	for (int i = 0; i < localVertexPositions.size(); i++)
		vertices[i].position = transform * glm::vec4(originalVertexPositions[i], 0.0f, 1.0f);
}
