#pragma once

#include <algorithm>

#include "Vertex.h"
#include "Shader.h"
#include "../Core/Object.h"
#include "../Core/Parameter.h"
#include "../Core/Log.h"

struct KeyformData
{
	glm::vec2 position;
	float rotation;
	glm::vec2 scale;
	std::unordered_map<int, glm::vec2> vertices;

	KeyformData(glm::vec2 p = glm::vec2(), float r = 0.0f, glm::vec2 s = glm::vec2())
	{
		position = p;
		rotation = r;
		scale = s;
	}

	~KeyformData() {}
};

class ModelPart : public Object
{
public:
	enum class PartType : char { model = 0, mesh = 1, divider = 2, warpDeformer = 3, rotationDeformer = 4 };

	PartType type = PartType::mesh;

	std::shared_ptr<ModelPart> parent;
	std::vector<std::shared_ptr<ModelPart>> children;

	//position in psd
	glm::vec2 originalPos = glm::vec2(0.0f);

	glm::vec2 basePos = glm::vec2(0.0f);
	float baseRotation = 0.0f;
	glm::vec2 baseScale = glm::vec2(1.0f, 1.0f);

	std::vector<std::string> paramNames;
	std::vector<std::vector<float>> paramKeyvalues;
	std::vector<float> paramWeights;

	std::vector<Vertex> vertices;
	std::vector<glm::vec2> localVertexPositions;
	std::vector<glm::vec2> originalVertexPositions;
	std::vector<unsigned int> indices;

	//std::map<std::string, std::vector<glm::vec2>> paramPos;
	std::vector<KeyformData> keyforms;
	std::vector<int> keyformsPerDimension;
	std::vector<int> keyformIndices;
	std::vector<float> keyformWeights;
	int keyformIndex = -1;

	void updateTransform(std::unordered_map<std::string, float>& paramValues);
	void warpTransform(glm::vec2 delta);

	virtual void modelUpdate(std::unordered_map<std::string, float>& paramValues) {}
	virtual void render() {}

	void addVertex(float xCoord, float yCoord);
	void addVertex(float xCoord, float yCoord, float xTexCoord, float yTexCoord);

	void clearMeshData();
	void updateVertexData();

	virtual void renderInspector() {}

	//returns -1 if not on keyform
	void calculateKeyformIndex(std::unordered_map<std::string, float>& paramValues);
	void removeKeyform(std::string paramName, float keyvalue);
	void removeParameter(std::string paramName);

	//move functions that use these stuff into derived classes later
	unsigned int getVao() { return vao; }
	unsigned int getVbo() { return vbo; }
	unsigned int getEbo() { return ebo; }

protected:
	unsigned int vao = 0, vbo = 0, ebo = 0;
};

