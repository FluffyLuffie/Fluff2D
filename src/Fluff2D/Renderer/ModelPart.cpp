#include "ModelPart.h"
#include "ModelMesh.h"

void ModelPart::updateTransform(std::unordered_map<std::string, float>& paramValues)
{
	//interpolate the values of the keyforms
	glm::vec2 finalPos = basePos;
	float finalRotation = baseRotation;
	glm::vec2 finalScale = baseScale;
	float finalRenderOrder = (float)baseRenderOrder;
	glm::vec4 finalColor = baseColor;

	//update based on parameters
	if (paramNames.size())
	{
		calculateKeyformIndex(paramValues);

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

		for (int i = 0; i < keyformWeights.size(); i++)
		{
			int keyformIndex = 0;
			for (int j = 0; j < paramWeights.size(); j++)
				keyformIndex += (bool((i & (1 << j))) + int(paramWeights[j])) * keyformsPerDimension[j];
			keyformIndex = std::min(keyformIndex, static_cast<int>(keyforms.size() - 1));
			keyformIndices[i] = keyformIndex;

			finalPos += keyforms[keyformIndex].deltaPosition * keyformWeights[i];
			finalRotation += keyforms[keyformIndex].deltaRotation * keyformWeights[i];
			finalScale += keyforms[keyformIndex].deltaScale * keyformWeights[i];
			finalRenderOrder += keyforms[keyformIndex].deltaRenderOrder * keyformWeights[i];
			finalColor += keyforms[keyformIndex].deltaColor * keyformWeights[i];
		}

		//for each vertex
		for (int i = 0; i < localVertexPositions.size(); i++)
		{
			glm::vec2 vPos = glm::vec2();

			//for each of the keyforms
			for (int j = 0; j < keyformIndices.size(); j++)
			{
				//if found
				if (keyforms[keyformIndices[j]].deltaVertices.find(i) != keyforms[keyformIndices[j]].deltaVertices.end())
					vPos += keyforms[keyformIndices[j]].deltaVertices[i] * keyformWeights[j];
			}

			localVertexPositions[i] = vPos + originalVertexPositions[i];
		}
	}
	else
		for (int i = 0; i < localVertexPositions.size(); i++)
			localVertexPositions[i] = originalVertexPositions[i];

	pos = finalPos;
	rotation = finalRotation;
	scale = finalScale;
	renderOrder = (int)finalRenderOrder;
	color = finalColor;

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
	localTransform = glm::translate(localTransform, glm::vec3(delta, 0.0f));
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

	//set tex coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
}

void ModelPart::calculateKeyformIndex(std::unordered_map<std::string, float>& paramValues)
{
	int index = 0;

	for (int i = 0; i < paramNames.size(); i++)
	{
		auto it = std::find(paramKeyvalues[i].begin(), paramKeyvalues[i].end(), paramValues[paramNames[i]]);
		//if paramValue not in vector
		if (it == paramKeyvalues[i].end())
		{
			keyformIndex = -1;
			return;
		}

		int keyPos = static_cast<int>(it - paramKeyvalues[i].begin());

		index += keyformsPerDimension[i] * keyPos;
	}

	keyformIndex = index;
}

void ModelPart::addKeyform(const std::string& paramName, float keyvalue)
{
	bool paramExists = false;

	int paramNameIndex;
	auto it = std::find(paramNames.begin(), paramNames.end(), paramName);

	//if first keyform for parameter name
	if (it == paramNames.end())
	{
		paramNameIndex = static_cast<int>(paramNames.size());
		paramNames.push_back(paramName);
	}
	else
	{
		paramNameIndex = static_cast<int>(it - paramNames.begin());
		paramExists = true;
	}

	paramKeyvalues.resize(paramNames.size());
	paramWeights.resize(paramNames.size());

	//if keyval already exists
	if (std::find(paramKeyvalues[paramNameIndex].begin(), paramKeyvalues[paramNameIndex].end(), keyvalue) != paramKeyvalues[paramNameIndex].end())
		return;

	paramKeyvalues[paramNameIndex].push_back(keyvalue);
	std::sort(paramKeyvalues[paramNameIndex].begin(), paramKeyvalues[paramNameIndex].end());

	keyformsPerDimension.resize(paramNames.size());
	int keyformInterpolateCount = 1, totalKeyformCount = 1, prevKeyformCount = static_cast<int>(keyforms.size());
	for (int i = 0; i < paramKeyvalues.size(); i++)
	{
		keyformInterpolateCount *= 2;
		keyformsPerDimension[i] = totalKeyformCount;
		totalKeyformCount *= static_cast<int>(paramKeyvalues[i].size());
	}
	keyformWeights.resize(keyformInterpolateCount);
	keyformIndices.resize(keyformInterpolateCount);

	//for first keyform, use this
	if (!keyforms.size())
	{
		keyforms.push_back(KeyformData(pos - basePos, rotation - baseRotation, scale - baseScale, renderOrder - baseRenderOrder, color - baseColor));
		return;
	}

	//don't need to do stuff if first keyvalue for a parameter
	if (!paramExists)
		return;

	//copy/move keyforms
	std::vector<KeyformData> newKeyforms;
	newKeyforms.reserve(totalKeyformCount);
	std::vector<int> paramCounter(paramKeyvalues.size() + 1, 0);

	int keyvalueIndex = static_cast<int>(std::find(paramKeyvalues[paramNameIndex].begin(), paramKeyvalues[paramNameIndex].end(), keyvalue) - paramKeyvalues[paramNameIndex].begin());
	for (int i = 0, oldI = 0; i < totalKeyformCount; i++)
	{
		//TODO: take into account neighbor keyvalues
		if (paramCounter[paramNameIndex] == keyvalueIndex)
		{
			newKeyforms.push_back(KeyformData());
		}
		else
		{
			newKeyforms.push_back(keyforms[oldI]);
			oldI++;
		}

		paramCounter[0]++;
		for (int j = 0; j < paramKeyvalues.size(); j++)
			if (paramCounter[j] == paramKeyvalues[j].size())
			{
				paramCounter[j] = 0;
				paramCounter[j + 1]++;
			}
	}

	keyforms = newKeyforms;
}

void ModelPart::removeKeyform(const std::string& paramName, int keyIndex)
{
	int paramIndex = static_cast<int>(std::find(paramNames.begin(), paramNames.end(), paramName) - paramNames.begin());

	if (paramKeyvalues[paramIndex].size() == 1)
	{
		removeParameter(paramName);
		return;
	}

	std::vector<int> paramCounter(paramKeyvalues.size() + 1, 0);
	std::vector<KeyformData> newKeyforms;

	for (int i = 0; i < keyforms.size(); i++)
	{
		if (paramCounter[paramIndex] != keyIndex)
			newKeyforms.push_back(keyforms[i]);

		paramCounter[0]++;
		for (int j = 0; j < paramKeyvalues.size(); j++)
			if (paramCounter[j] == paramKeyvalues[j].size())
			{
				paramCounter[j] = 0;
				paramCounter[j + 1]++;
			}
	}

	paramKeyvalues[paramIndex].erase(paramKeyvalues[paramIndex].begin() + keyIndex);
	keyforms = newKeyforms;
}

void ModelPart::removeParameter(const std::string& paramName)
{
	std::cout << "removeParameter not done" << std::endl;
}
