#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "ModelPart.h"
#include "../UI/ModelPartUI.h"
#include "../Core/Parameter.h"
#include "Triangulator.h"

class ModelMesh : public ModelPart
{
public:
	inline static const char* blendModes[] = { "normal", "add", "multiply" };

	ModelMesh();
	~ModelMesh();

	int textureIndex = 0;

	bool invertClip = false;
	std::vector<std::string> clipMeshes;

	//0 is normal, 1 is add, 2 is multiply
	int blendMode = 0;

	void render() override;
	void renderInspector() override;

	void addMeshVertex(glm::vec2 vPos, int atlasWidth, int atlasHeight);

	void createBasicMesh(int layerX, int layerY, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight);

	//testing, call after making basic mesh
	void createTriMesh(int boxCountX, int boxCountY, int atlasWidth, int atlasHeight);

	void startMeshEdit();
	void removeVertex(int index);
	void autoMesh(std::filesystem::path directoryPath, int atlasWidth, int atlasHeight, int edgeOut, int edgeIn, int edgeSpacing, int insideSpacing, unsigned char threshold);
	void removeInvisibleTriangles(std::filesystem::path directoryPath, unsigned char threshold);

	glm::vec2 posToTexCoord(const glm::vec2& vPos, int atlasWidth, int atlasHeight);

private:
	int atlasPositionX = 0, atlasPositionY = 0, textureWidth = 0, textureHeight = 0;
	bool flipped = false;
};

