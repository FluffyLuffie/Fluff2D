#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Vertex.h"
#include "ModelPart.h"
#include "Shader.h"

class ModelMesh : public ModelPart
{
public:
	ModelMesh();
	~ModelMesh();

	int layerOrder = 0;
	glm::vec4 color;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	bool flipped = false;

	void loadFromImage(const char* filePath);
	void clearMeshData();

	void update() override;
	void renderMesh();

	void createBasicMesh();
	void createBasicMesh(int layerY, int layerX, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight);
};

