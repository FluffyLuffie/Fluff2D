#include "Model.h"

Model::Model()
{
}

Model::~Model()
{
}

void Model::loadTextureAtlas(const char* filePath)
{
	std::cout << "Model::loadTextureAtlas not done yet don't call" << std::endl;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data = stbi_load(filePath, &atlasWidth, &atlasHeight, &atlasNrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture atlas" << std::endl;
	}
	stbi_image_free(data);
}

void Model::addMesh(const char* filePath)
{
	std::unique_ptr<ModelMesh> newMesh = std::make_unique<ModelMesh>();
	newMesh->loadFromImage(filePath);
	partList.push_back(std::move(newMesh));
}

void Model::update()
{
	//std::cout << "updating model" << std::endl;
	for (auto it = partList.begin(); it != partList.end(); ++it)
	{
		(*it)->update();
	}
}

void Model::createNewModel(const char* filePath, const std::vector<Rectangle>& rects)
{

}
