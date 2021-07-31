#include "ModelMesh.h"

ModelMesh::ModelMesh()
{
}

ModelMesh::~ModelMesh()
{
}

void ModelMesh::loadFromImage(const char* filePath)
{
	/*
	TextureLoader::loadTexture(&textureID, filePath, &textureWidth, &textureHeight, &textureNrChannels);

	//prob change this to not bind texture here
	glBindTexture(GL_TEXTURE_2D, textureID);

	//testing idk, make simple box
	createBasicMesh();

	generateMeshData();
	*/
}

void ModelMesh::clearMeshData()
{
	vertices.clear();
	indices.clear();
}

void ModelMesh::update()
{
	//testing stuff
	if (Event::keyDown(GLFW_KEY_RIGHT))
		vertices[0].position.x += 4;
	if (Event::keyDown(GLFW_KEY_LEFT))
		vertices[0].position.x -= 4;
	if (Event::keyDown(GLFW_KEY_UP))
		vertices[0].position.y += 4;
	if (Event::keyDown(GLFW_KEY_DOWN))
		vertices[0].position.y -= 4;
	if (Event::keyDown(GLFW_KEY_R))
		vertices[0].position = glm::vec2(0.0f);

	//if (parent)
	//	parent->deformEffect(transform);
}

void ModelMesh::renderInspector()
{
	ImGui::Text(name.c_str());
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

//testing purpose only, don't use
void ModelMesh::createBasicMesh()
{
	clearMeshData();

	//create default vertices
	vertices.push_back(Vertex(0.0f, 0.0f, 0.5f, 0.5f));
	vertices.push_back(Vertex(-500.0f, 500.0f, 0.0f, 1.0f));
	vertices.push_back(Vertex(500.0f, 500.0f, 1.0f, 1.0f));
	vertices.push_back(Vertex(500.0f, -500.0f, 1.0f, 0.0f));
	vertices.push_back(Vertex(-500.0f, -500.0f, 0.0f, 0.0f));

	//make indices
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

	pos.x = 0.0f;
	pos.y = 0.0f;

	color = glm::vec4(1.0f);
}

void ModelMesh::createBasicMesh(int layerY, int layerX, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight)
{
	clearMeshData();

	//create default vertices
	//order is center, top left, top right, bottom right, bottom left (basically clockwise)
	if (flip)
	{
		flipped = flip;
		vertices.push_back(Vertex(0.0f, 0.0f, (layerX + layerW / 2.0f) / atlasWidth, (atlasHeight - layerY - layerH / 2.0f) / atlasHeight));
		vertices.push_back(Vertex(layerH / -2.0f, layerW / 2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight));
		vertices.push_back(Vertex(layerH / 2.0f, layerW / 2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight));
		vertices.push_back(Vertex(layerH / 2.0f, layerW / -2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight));
		vertices.push_back(Vertex(layerH / -2.0f, layerW / -2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight));
	}
	else
	{
		vertices.push_back(Vertex(0.0f, 0.0f, (layerX + layerW / 2.0f) / atlasWidth, (atlasHeight - layerY - layerH / 2.0f) / atlasHeight));
		vertices.push_back(Vertex(layerW / -2.0f, layerH / 2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight));
		vertices.push_back(Vertex(layerW / 2.0f, layerH / 2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY) / atlasHeight));
		vertices.push_back(Vertex(layerW / 2.0f, layerH / -2.0f, (float)(layerX + layerW) / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight));
		vertices.push_back(Vertex(layerW / -2.0f, layerH / -2.0f, (float)layerX / atlasWidth, (float)(atlasHeight - layerY - layerH) / atlasHeight));
	}

	//make indices
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

	color = glm::vec4(1.0f);
}
