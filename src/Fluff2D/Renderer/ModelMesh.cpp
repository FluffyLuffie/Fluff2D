#include "ModelMesh.h"

ModelMesh::ModelMesh()
{
	//change shader later
	shader = std::make_unique<Shader>("saves/shaders/shader.vs", "saves/shaders/shader.fs");
}

ModelMesh::~ModelMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void ModelMesh::loadFromImage(const char* filePath)
{
	TextureLoader::loadTexture(&textureID, filePath, &textureWidth, &textureHeight, &textureNrChannels);

	//prob change this to not bind texture here
	glBindTexture(GL_TEXTURE_2D, textureID);

	//testing idk, make simple box
	createBasicMesh();

	generateMeshData();
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

	renderMesh();
}

void ModelMesh::renderMesh()
{
	//std::cout << "modelMesh render" << std::endl;
	shader->use();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void ModelMesh::generateMeshData()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

	//set vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	//set tex coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ModelMesh::createBasicMesh()
{
	clearMeshData();

	//create default vertices
	vertices.push_back(Vertex(0.0f, 0.0f, 0.5f, 0.5f));
	vertices.push_back(Vertex(-0.5f, 0.5f, 0.0f, 1.0f));
	vertices.push_back(Vertex(0.5f, 0.5f, 1.0f, 1.0f));
	vertices.push_back(Vertex(0.5f, -0.5f, 1.0f, 0.0f));
	vertices.push_back(Vertex(-0.5f, -0.5f, 0.0f, 0.0f));

	/*
	//make vertices, just a square right now, make it scale with image size later
	vertices.push_back(glm::vec2(0.0f, 0.0f)); //middle
	vertices.push_back(glm::vec2(-0.5f, 0.5f)); //top left
	vertices.push_back(glm::vec2(0.5f, 0.5f)); //top right
	vertices.push_back(glm::vec2(0.5f, -0.5f)); //bottom right
	vertices.push_back(glm::vec2(-0.5f, -0.5f)); //bottom left

	//make texture coordinates
	texCoords.push_back(glm::vec2(0.5f, 0.5f));
	texCoords.push_back(glm::vec2(0.0f, 1.0f));
	texCoords.push_back(glm::vec2(1.0f, 1.0f));
	texCoords.push_back(glm::vec2(1.0f, 0.0f));
	texCoords.push_back(glm::vec2(0.0f, 0.0f));
	*/

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
}
