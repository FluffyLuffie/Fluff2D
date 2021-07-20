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
	//std::cout << "modelMesh update" << std::endl;

	/*
	//sends updated vertex position to GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	//testing, position 249 is half of 500
	glBufferSubData(GL_ARRAY_BUFFER, 249, sizeof(texCoords), &texCoords[0]);

	//sends updated indices to GPU
	glBindBuffer(GL_ARRAY_BUFFER, EBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(indices), &indices[0]);
	*/

	//renderMesh();
}

void ModelMesh::renderMesh()
{
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
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
