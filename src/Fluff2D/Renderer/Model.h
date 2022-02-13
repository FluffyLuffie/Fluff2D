#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

#include "RenderGroup.h"
#include "WarpDeformer.h"
#include "RotationDeformer.h"
#include "LayerRect.h"
#include "../Core/Settings.h"
#include "Camera2D.h"

#include "stb_image/stb_image_write.h"

struct VertexSpecifier
{
	std::string partName;
	int index;

	VertexSpecifier(std::string name, int i) { partName = name, index = i; }
	~VertexSpecifier() {}

	friend bool operator == (const VertexSpecifier& v1, const VertexSpecifier& v2) { return v1.partName == v2.partName && v1.index == v2.index; }
};

class Model : public ModelPart
{
public:
	Model();
	~Model();

	glm::ivec2 psdDimension = glm::ivec2(0);

	float scale = 1.0f;
	glm::mat4 projection = glm::mat4(1.0f);

	unsigned int textureID = 0;
	Shader shader;

	std::vector<std::shared_ptr<ModelPartUI>> layerStructure;
	std::vector<std::shared_ptr<ModelPartUI>> modelStructure;

	std::unordered_map<std::string, std::shared_ptr<ModelPart>> partMap;
	std::vector<std::shared_ptr<ModelMesh>> modelMeshes;
	std::unordered_map<std::string, std::shared_ptr<ModelMesh>> meshMap;

	std::vector<VertexSpecifier> selectedVertices;
	std::unordered_map<VertexSpecifier*, glm::vec2> initialVerticesPos;

	int atlasWidth = 0, atlasHeight = 0, atlasNrChannels = 0;

	int mouseHoveredID = -1;
	bool screenshot = false;

	void update() override;
	void render() override;

	void generateDefaltParams();

	void reset();

	void renderMeshVertice(const std::string &meshName);
	void renderSelectedVertices();
	void moveSelectedVertices(const ImVec2 &originalMouseCoord);
	void updateOriginalVertexPositions();
	void renderClosestVertex(const std::string &partName, int vertexIndex);

	void generateTestBoxMesh(std::string partName, int boxSizeX, int boxSizeY);

	bool checkNameExists(const std::string &name);
	bool checkSameParent(const std::vector<std::string>& selectedParts);
	void addWarpDeformer(std::string name, const std::vector<std::string>& selectedParts, int countX, int countY);
	void addRotationDeformer(std::string name, const std::vector<std::string>& selectedParts);

	void resetParams();
	void updatePartMap();

	int findClosestVertex(const std::vector<std::string> &selectedParts, int *partNum);

	void updateCanvasCoord();

	void showMeshClippingingMenu(const std::string &meshName);

	void bindUniformTextures();

private:
	void updatePartMapRecursive(std::shared_ptr<ModelPart> part);

	unsigned int modelFbo = 0, modelTexColorBuffer = 0, mousePickBuffer = 0, modelRbo = 0;
	unsigned int maskVao = 0, maskVbo = 0, maskEbo = 0;

	Vertex maskVertices[4] = { Vertex(-1.0f, 1.0f, 0.5f, 1.0f), Vertex(1.0f, 1.0f, 1.0f, 1.0f) , Vertex(1.0f, -1.0f, 1.0f, 0.0f) , Vertex(-1.0f, -1.0f, 0.5f, 0.0f) };

	unsigned int canvasVao = 0, canvasVbo = 0, canvasEbo = 0;
	glm::vec2 canvasCoords[4] = { glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f) };

	unsigned int rectLineIndices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };

	std::multimap<int, int> renderOrderMap;

	void updateFrameBufferSize();

	void renderMaskedMesh(int meshNum);
};

