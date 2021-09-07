#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ModelPart.h"
#include "../UI/ModelPartUI.h"
#include "../Core/Parameter.h"

class ModelMesh : public ModelPart
{
public:
	ModelMesh();
	~ModelMesh();

	int layerOrder = 0;
	glm::vec4 color = glm::vec4(1.0f);

	std::vector<std::string> maskedMeshes;
	int maskedCount = 0;

	bool visible = true;
	int renderOrder = 0;

	void update() override;
	void secondUpdate() override;
	void render() override;
	void renderInspector() override;

	void createBasicMesh(int layerX, int layerY, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight);

	//testing, call after making basic mesh
	void createBoxMesh(int boxCountX, int boxCountY, int atlasWidth, int atlasHeight);

private:
	int atlasPositionX = 0, atlasPositionY = 0, textureWidth = 0, textureHeight = 0;
	bool flipped = false;
};

