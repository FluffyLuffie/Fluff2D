#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "ModelMesh.h"
#include "WarpDeformer.h"
#include "RotationDeformer.h"
#include "LayerRect.h"
#include "../Core/Settings.h"
#include "Camera2D.h"

class Model : public ModelPart
{
public:
	Model();
	~Model();

	glm::ivec2 psdDimension = glm::ivec2(0);

	float scale = 1.0f;
	glm::mat4 projection = glm::mat4(1.0f);

	unsigned int textureID = 0;
	Shader meshShader, screenShader;

	std::vector<std::shared_ptr<ModelPartUI>> layerStructure;

	std::unordered_map<std::string, std::shared_ptr<ModelPart>> partMap;
	std::vector<std::shared_ptr<ModelMesh>> modelMeshes;

	std::vector<std::string> paramNames;
	std::unordered_map<std::string, std::shared_ptr<Parameter>> paramMap;

	std::map<Vertex*, std::string> selectedVertices;
	std::unordered_map<Vertex*, glm::vec2> initialVerticesPos;

	int atlasWidth = 0, atlasHeight = 0, atlasNrChannels = 0;

	void update() override;
	void render() override;

	void generateDefaltParams();

	void reset();

	void renderMeshVertice(const std::string &meshName);
	void renderSelectedVertices();
	void moveSelectedVertices(const ImVec2 &originalMouseCoord);
	void updateOriginalVertexPositions();
	void renderClosestVertex(Vertex* closestVert, const std::string &partName);

	void generateTestBoxMesh(std::string partName, int boxSizeX, int boxSizeY);

	bool checkNameExists(const std::string &name);
	bool checkSameParent(const std::vector<std::string>& selectedParts);
	void addWarpDeformer(std::string name, const std::vector<std::string>& selectedParts, int countX, int countY);
	void addRotationDeformer(std::string name, const std::vector<std::string>& selectedParts);

	void resetParams();
	void updatePartMap();

	Vertex* findClosestVertex(const std::vector<std::string> &selectedParts, int *partNum);

	void updateCanvasCoord();
	void updateFrameBufferSize();

private:
	void updatePartMapRecursive(std::shared_ptr<ModelPart> part);

	unsigned int fbo = 0, texColorBuffer = 0, rbo = 0;

	unsigned int canvasVao = 0, canvasVbo = 0, canvasEbo = 0;
	glm::vec2 canvasCoords[4] = { glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f) };
	unsigned int canvasIndices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };

};

