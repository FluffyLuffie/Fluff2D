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

	unsigned int textureID = 0;
	Shader shader;

	std::vector<std::shared_ptr<ModelPartUI>> layerStructure;

	std::unordered_map<std::string, std::shared_ptr<ModelPart>> partMap;
	std::vector<std::shared_ptr<ModelMesh>> modelMeshes;
	std::unordered_map<std::string, std::shared_ptr<ModelMesh>> meshMap;

	std::vector<VertexSpecifier> selectedVertices;
	std::unordered_map<VertexSpecifier*, glm::vec2> initialDragData;

	std::unordered_map<std::string, float> paramValues;
	std::unordered_map<std::string, std::shared_ptr<Parameter>> paramMap;

	int atlasWidth = 0, atlasHeight = 0, atlasNrChannels = 0;

	bool detectMouseHover = false;
	int mouseHoveredID = -1;
	bool screenshot = false;

	unsigned int modelTexColorBuffer = 0, modelTexTrueColorBuffer = 0;
	glm::vec2 fbDimension = glm::vec2();
	int fbX = 0, fbY = 0;

	void update() override;
	void render() override;

	void renderEditMesh(const std::string& meshName);

	void generateDefaltParams();

	void reset();

	void renderMeshVertices(const std::string &meshName);
	void renderHighlightedMesh();
	void renderSelectedVertices();
	void forceRenderVertices(const std::string& meshName);
	void moveSelectedVertices(const glm::vec2 &originalMouseCoord, int dragMod);
	void moveMeshVertices(const glm::vec2& originalMouseCoord, int dragMod);
	void updateOriginalVertexPositions();
	void updateOriginalMeshPositions();
	void renderClosestVertex(const std::string &partName, int vertexIndex);

	void generateTestBoxMesh(std::string partName, int boxSizeX, int boxSizeY);

	bool checkNameExists(const std::string &name);
	bool checkSameParent(const std::vector<std::string>& selectedParts);
	void addWarpDeformer(std::string name, const std::vector<std::string>& selectedParts, int countX, int countY);
	void addRotationDeformer(std::string name, const std::vector<std::string>& selectedParts);

	void resetParams();
	void calculateModelParam();
	void addKeyform(const std::string& partName, const std::string& paramName, float keyvalue);

	void updatePartMap();

	int findClosestVertex(const std::vector<std::string> &selectedParts, int *partNum);

	void updateFrameBufferSize(int x, int y);
	void updateCanvasCoord();

	void showMeshClippingMenu(const std::string &meshName);

	void bindUniformTextures();

private:
	void updatePartMapRecursive(std::shared_ptr<ModelPart> part);

	unsigned int modelFbo = 0, modelTrueFbo = 0, mousePickBuffer = 0;
	unsigned int canvasVao = 0, canvasVbo = 0, canvasEbo = 0;
	unsigned int selectedVertexVao = 0, selectedVertexVbo = 0, selectedVertexEbo = 0;
	unsigned int closestVertexVao = 0, closestVertexVbo = 0, closestVertexEbo = 0;
	glm::vec2 canvasCoords[4] = { glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f) };

	//also used to just render a point with index 0
	unsigned int rectLineIndices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };

	std::multimap<int, int> renderOrderMap;

	GLenum bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	void renderMaskedMesh(int meshNum);
};

