#include "ModelPart.h"

void ModelPart::updateTransform(std::unordered_map<std::string, float>& paramValues)
{
	//update based on parameters
	if (paramNames.size())
	{
		//for each parameter assigned to part, find weights
		for (int i = 0; i < paramNames.size(); i++)
		{
			//if value below min keyvalue
			if (paramValues[paramNames[i]] <= paramKeyvalues[i][0])
			{
				paramWeights[i] = 0.0f;
				continue;
			}
			//if value above max keyvalue
			if (paramValues[paramNames[i]] >= paramKeyvalues[i][paramKeyvalues[i].size() - 1])
			{
				paramWeights[i] = static_cast<float>(paramKeyvalues[i].size() - 1);
				continue;
			}
			//find which 2 key values the param is between
			bool found = false;
			for (int j = 0; !found && j < paramKeyvalues[i].size() - 1; j++)
			{
				if (paramValues[paramNames[i]] <= paramKeyvalues[i][j + 1])
				{
					//integer part represents the lower index
					//fractional part represents the distance from the lower index
					paramWeights[i] = j + (paramValues[paramNames[i]] - paramKeyvalues[i][j]) / (paramKeyvalues[i][j + 1] - paramKeyvalues[i][j]);
					found = true;
				}
			}
		}

		/*
		glm::vec2 finalPos = paramPos["headX"][static_cast<int>(paramWeights["headX"])] * (1.0f - std::fmod(paramWeights["headX"], 1.0f));
		//check if next keyvalue exists, and add if yes
		if (paramValues["headX"] < paramKeyvalues["headX"][paramKeyvalues["headX"].size() - 1])
			finalPos += paramPos["headX"][static_cast<int>(paramWeights["headX"]) + 1] * std::fmod(paramWeights["headX"], 1.0f);
		*/

		//calculate how much weight each keyform has
		for (int i = 0; i < keyformWeights.size(); i++)
		{
			float mult = 1.0f;
			for (int j = 0; j < paramWeights.size(); j++)
				mult *= bool((i & (1 << j))) - std::fmod(paramWeights[j], 1.0f);

			keyformWeights[i] = std::abs(mult);
		}

		//glm::vec2 finalPos = glm::vec2();
		//pos = finalPos;
	}

	//update local transform
	localTransform = glm::mat4(1.0f);
	localTransform = glm::translate(localTransform, glm::vec3(pos, 0.0f));
	localTransform = glm::rotate(localTransform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	localTransform = glm::scale(localTransform, glm::vec3(scale, 1.0f));

	//assume parent always exists
	transform = parent->transform * localTransform;

	for (int i = 0; i < localVertexPositions.size(); i++)
		vertices[i].position = transform * glm::vec4(localVertexPositions[i], 0.0f, 1.0f);
}

void ModelPart::warpTransform(glm::vec2 delta)
{
	localTransform = glm::mat4(1.0f);
	localTransform = glm::translate(localTransform, glm::vec3(pos + delta, 0.0f));
	localTransform = glm::rotate(localTransform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	localTransform = glm::scale(localTransform, glm::vec3(scale, 1.0f));

	//assume parent always exists
	transform = parent->transform * localTransform;
}

void ModelPart::addVertex(float xCoord, float yCoord)
{
	vertices.emplace_back(xCoord, yCoord);
	localVertexPositions.emplace_back(xCoord, yCoord);
	originalVertexPositions.emplace_back(xCoord, yCoord);
}

void ModelPart::addVertex(float xCoord, float yCoord, float xTexCoord, float yTexCoord)
{
	vertices.emplace_back(xCoord, yCoord, xTexCoord, yTexCoord);
	localVertexPositions.emplace_back(xCoord, yCoord);
	originalVertexPositions.emplace_back(xCoord, yCoord);
}

void ModelPart::clearMeshData()
{
	vertices.clear();
	localVertexPositions.clear();
	originalVertexPositions.clear();
	indices.clear();
}

void ModelPart::updateVertexData()
{
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

	//set vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	//set tex coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);
}
