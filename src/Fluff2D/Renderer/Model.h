#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>

#include "ModelMesh.h"
#include "LayerRect.h"
#include "../Core/Window.h"
#include "../UI/ModelPartUI.h"

class Model
{
public:
	Model();
	~Model();

	//DELETE LATER
	bool testModelRotation = false;

	glm::vec2 modelPos;
	float scale = 1.0f;
	float rotation = 0.0f;

	unsigned int textureID = 0;
	Shader shader;

	ModelPartUI rootPart;
	std::vector<ModelMesh> meshList;
	std::map<std::string, float> paramList;

	int atlasWidth, atlasHeight, atlasNrChannels;

	void update();
	void render();

	void setShader();
	void updateMeshNumber();

	//testing functions
	void printPartsList();

private:
	std::vector<unsigned int> VAO, VBO, EBO;
};

