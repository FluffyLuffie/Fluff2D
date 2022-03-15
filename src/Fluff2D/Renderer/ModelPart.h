#pragma once

#include "Vertex.h"
#include "Shader.h"
#include "../Core/Object.h"
#include "../Core/Parameter.h"
#include "../Core/Log.h"

class ModelPart : public Object
{
public:
	enum class PartType : char { model = 0, mesh = 1, divider = 2, warpDeformer = 3, rotationDeformer = 4 };

	PartType type = PartType::mesh;

	std::shared_ptr<ModelPart> parent;
	std::vector<std::shared_ptr<ModelPart>> children;

	glm::vec2 originalPos = glm::vec2(0.0f);
	float originalRotation = 0.0f;

	std::vector<std::string> paramNames;
	std::unordered_map<std::string, std::shared_ptr<Parameter>> paramMap;

	std::vector<Vertex> vertices;
	std::vector<glm::vec2> localVertexPositions;
	std::vector<glm::vec2> originalVertexPositions;
	std::vector<glm::vec2> deltaVertexPositions;
	std::vector<unsigned int> indices;

	void updateTransform();
	void warpTransform(glm::vec2 delta);

	virtual void render() {}

	void addVertex(float xCoord, float yCoord);
	void addVertex(float xCoord, float yCoord, float xTexCoord, float yTexCoord);

	void clearMeshData();
	void updateVertexData();

	virtual void renderInspector() {}

	//move functions that use these stuff into derived classes later
	unsigned int getVao() { return vao; }
	unsigned int getVbo() { return vbo; }
	unsigned int getEbo() { return ebo; }

protected:
	unsigned int vao = 0, vbo = 0, ebo = 0;
};

