#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include "Fluff2D/Renderer/ModelMesh.h"
#include "Fluff2D/Core/Rectangle.h"

class Model
{
public:
	Model();
	~Model();

	unsigned int textureID = 0;
	std::vector<std::unique_ptr<ModelPart>> partList;
	//std::vector<std::unique_ptr<ModelMesh>> meshList;
	std::map<std::string, float> paramList;

	void update();

	void loadTextureAtlas(const char* filePath);
	void addMesh(const char* filePath);
	void createNewModel(const char* filePath, const std::vector<Rectangle>& rects);
private:
	int atlasWidth, atlasHeight, atlasNrChannels;
};

