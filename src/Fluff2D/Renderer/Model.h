#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>

#include "ModelMesh.h"
#include "LayerRect.h"
#include "../Core/Settings.h"

class Model
{
public:
	Model();
	~Model();

	glm::ivec2 psdDimension = glm::ivec2(0);

	glm::vec2 modelPos = glm::vec2(0.0f);
	float scale = 1.0f;
	float rotation = 0.0f;
	glm::mat4 projection = glm::mat4(1.0f);

	unsigned int textureID = 0;
	Shader shader;

	std::vector<std::shared_ptr<ModelPartUI>> layerStructure;
	std::vector<std::shared_ptr<ModelPartUI>> meshStructure;

	std::unordered_map<std::string, std::shared_ptr<ModelPart>> partMap;
	std::vector<std::shared_ptr<ModelPart>> modelParts;
	std::vector<std::shared_ptr<ModelMesh>> modelMeshes;

	std::vector<std::string> paramNames;
	std::unordered_map<std::string, std::shared_ptr<Parameter>> paramMap;

	int atlasWidth = 0, atlasHeight = 0, atlasNrChannels = 0;

	void update();
	void render();

	void updateMeshNumber();
	void generateDefaltParams();

	void reset();

	//testing functions
	void renderMeshVertice(int meshNum);
	void renderMeshVertice(const std::string &meshName);

	void resetParams();

	Vertex* findClosestVertex(const std::vector<std::string> &selectedParts);

private:
	std::vector<unsigned int> VAO, VBO, EBO;

	void updatePartMap(std::shared_ptr<ModelPart> part);
};

