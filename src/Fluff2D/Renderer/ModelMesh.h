#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Vertex.h"
#include "Deformer.h"
#include "Shader.h"
#include "../Core/Window.h"
#include "../UI/ModelPartUI.h"
#include "../Core/Parameter.h"

class ModelMesh : public ModelPart
{
public:
	ModelMesh();
	~ModelMesh();

	int layerOrder = 0;
	glm::vec4 color = glm::vec4(1.0f);

	glm::vec2 originalPos = glm::vec2(0.0f);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	bool flipped = false;

	void loadFromImage(const char* filePath);
	void clearMeshData();

	void update() override;
	void renderInspector() override;

	void createBasicMesh();
	void createBasicMesh(int layerY, int layerX, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight);
};

