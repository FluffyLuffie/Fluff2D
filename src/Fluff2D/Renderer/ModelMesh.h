#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Fluff2D/Renderer/Vertex.h"
#include "Fluff2D/Renderer/ModelPart.h"
#include "Fluff2D/Renderer/TextureLoader.h"
#include "Fluff2D/Renderer/Shader.h"

class ModelMesh : public ModelPart
{
public:
	ModelMesh();
	~ModelMesh();

	int layerOrder = 0;

	unsigned int textureID;
	int textureWidth = 0;
	int textureHeight = 0;
	int textureNrChannels = 0;

	//std::vector<Vertex> vertices;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	void loadFromImage(const char* filePath);
	void clearMeshData();

	void update() override;
	void renderMesh();

	void generateMeshData();
	void createBasicMesh();

private:
	unsigned int VAO = 0, VBO = 0, EBO = 0;
	std::unique_ptr<Shader> shader;
};

