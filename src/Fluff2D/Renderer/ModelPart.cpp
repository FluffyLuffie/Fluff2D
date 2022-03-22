#include "ModelPart.h"

void ModelPart::updateTransform(std::unordered_map<std::string, float>& paramValues)
{
	//update based on parameters
	if (paramNames.size())
	{
		//for each parameter assigned to part, find weights
		for (int i = 0; i < paramNames.size(); i++)
		{
			float clampedVal = std::clamp(paramValues[paramNames[i]], paramKeyvalues[i][0], paramKeyvalues[i][paramKeyvalues[i].size() - 1]);

			//find which 2 key values the param is between
			bool found = false;
			for (int j = 0; !found && j < paramKeyvalues[i].size() - 1; j++)
			{
				if (clampedVal <= paramKeyvalues[i][j + 1])
				{
					//integer part represents the lower index
					//fractional part represents the distance from the lower index
					paramWeights[i] = j + (clampedVal - paramKeyvalues[i][j]) / (paramKeyvalues[i][j + 1] - paramKeyvalues[i][j]);
					found = true;
				}
			}
		}

		//calculate how much weight each keyform has
		for (int i = 0; i < keyformWeights.size(); i++)
		{
			float mult = 1.0f;
			for (int j = 0; j < paramWeights.size(); j++)
				mult *= bool((i & (1 << j))) - (1.0f - std::fmod(paramWeights[j], 1.0f));

			keyformWeights[i] = std::abs(mult);
		}

		//interpolate the values of the keyforms
		glm::vec2 finalPos = glm::vec2();
		float finalRotation = 0.0f;
		glm::vec2 finalScale = glm::vec2();

		for (int i = 0; i < keyformWeights.size(); i++)
		{
			int keyformIndex = 0;
			for (int j = 0; j < paramWeights.size(); j++)
				keyformIndex += (bool((i & (1 << j))) + int(paramWeights[j])) * keyformsPerDimension[j];
			keyformIndex = std::min(keyformIndex, static_cast<int>(keyforms.size() - 1));
			keyformIndices[i] = keyformIndex;

			finalPos += keyforms[keyformIndex].position * keyformWeights[i];
			finalRotation += keyforms[keyformIndex].rotation * keyformWeights[i];
			finalScale += keyforms[keyformIndex].scale * keyformWeights[i];
		}

		pos = finalPos;
		rotation = finalRotation;
		scale = finalScale;

		//for each vertex
		for (int i = 0; i < localVertexPositions.size(); i++)
		{
			glm::vec2 vPos = glm::vec2();
			float leftoverWeight = 1.0f;

			//for each of the keyforms
			for (int j = 0; j < keyformIndices.size(); j++)
			{
				//if found
				if (keyforms[keyformIndices[j]].vertices.find(i) != keyforms[keyformIndices[j]].vertices.end())
				{
					vPos += keyforms[keyformIndices[j]].vertices[i] * keyformWeights[j];
					leftoverWeight -= keyformWeights[j];
				}
			}

			//leftover weight is the original position's weight
			vPos += originalVertexPositions[i] * leftoverWeight;

			localVertexPositions[i] = vPos;
		}
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
