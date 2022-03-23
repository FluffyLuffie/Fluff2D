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

	if (dataChanged && keyformIndex != -1)
	{
		keyforms[keyformIndex].position = pos;
		keyforms[keyformIndex].rotation = rotation;
		keyforms[keyformIndex].scale = scale;
	}

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
}

void ModelMesh::createTriMesh(int boxCountX, int boxCountY, int atlasWidth, int atlasHeight)
{
	clearMeshData();

	//create vertices for boxes
	vertices.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);
	localVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);
	originalVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);

	//TODO: add flipped
	if (flipped)
	{

	}
	else
	{
		//normal box
		for (int y = 0; y <= boxCountY; y++)
			for (int x = 0; x <= boxCountX; x++)
				addVertex(textureWidth * (-0.5f + 1.0f / boxCountX * x), textureHeight * (-0.5f + 1.0f / boxCountY * y), (atlasPositionX + ((float)textureWidth) * x / boxCountX) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight + ((float)textureHeight) * y / boxCountY) / atlasHeight);

		//put dot in middle
		for (int y = 0; y < boxCountY; y++)
			for (int x = 0; x < boxCountX; x++)
				addVertex(textureWidth * (-0.5f + 1.0f / boxCountX * (x + 0.5f)), textureHeight * (-0.5f + 1.0f / boxCountY * (y + 0.5f)), (atlasPositionX + ((float)textureWidth) * (x + 0.5f) / boxCountX) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight + ((float)textureHeight) * (y + 0.5f) / boxCountY) / atlasHeight);
	}

	Triangulator::triangulate(vertices, indices, atlasPositionX, atlasPositionY, flipped, atlasWidth, atlasHeight);
}
