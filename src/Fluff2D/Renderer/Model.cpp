#include "Model.h"
Model::Model()
{
	type = ModelPart::PartType::model;

	shader.setShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
	shader.use();

	shader.setInt("atlasTex", 0);
	shader.setInt("modelTex", 1);
	shader.setInt("mode", 0);

	//set rendered models quad
	vertices.reserve(4);
	vertices.emplace_back(-1.0f, 1.0f, 0.0f, 1.0f);
	vertices.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
	vertices.emplace_back(1.0f, -1.0f, 1.0f, 0.0f);
	vertices.emplace_back(-1.0f, -1.0f, 0.0f, 0.0f);

	indices.reserve(6);
	indices.emplace_back(0);
	indices.emplace_back(1);
	indices.emplace_back(2);
	indices.emplace_back(0);
	indices.emplace_back(2);
	indices.emplace_back(3);

	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

	glGenVertexArrays(1, (GLuint*)(&canvasVao));
	glGenBuffers(1, (GLuint*)(&canvasVbo));
	glGenBuffers(1, (GLuint*)(&canvasEbo));

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);

	//framebuffer stuff
	glGenFramebuffers(1, &modelFbo);
	glGenRenderbuffers(1, &modelRbo);
	glGenTextures(1, &modelTexColorBuffer);
	glGenTextures(1, &mousePickBuffer);

	updateFrameBufferSize();

	//masking stuff
	glGenVertexArrays(1, (GLuint*)(&maskVao));
	glGenBuffers(1, (GLuint*)(&maskVbo));
	glGenBuffers(1, (GLuint*)(&maskEbo));

	glBindVertexArray(maskVao);
	glBindBuffer(GL_ARRAY_BUFFER, maskVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, maskVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, maskEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);

}

Model::~Model() 
{
	glDeleteTextures(1, &textureID);

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	glDeleteVertexArrays(1, &canvasVao);
	glDeleteBuffers(1, &canvasVbo);
	glDeleteBuffers(1, &canvasEbo);

	glDeleteFramebuffers(1, &modelFbo);
	glDeleteRenderbuffers(1, &modelRbo);
	glDeleteTextures(1, &modelTexColorBuffer);
	glDeleteTextures(1, &mousePickBuffer);

	glDeleteVertexArrays(1, &maskVao);
	glDeleteBuffers(1, &maskVbo);
	glDeleteBuffers(1, &maskEbo);
}

void Model::generateDefaltParams()
{
	Log::logInfo("Generating default parameters");

	paramNames = {
		"headX", "headY", "headZ",
		"eyeROpen", "eyeLOpen", "eyeX", "eyeY",
		"browRY", "browLY", "browRX", "browLX", "browShapeR", "browShapeL",
		"mouthOpen", "mouthShape",
		"bodyX", "bodyY", "bodyZ",
		"breath"
	};

	for (int i = 0; i < paramNames.size(); i++)
	{
		switch (i)
		{
		case 0: case 1: case 2:
			paramMap[paramNames[i]] = std::make_shared<Parameter>(30.0f);
			break;
		case 3: case 4: case 13: case 18:
			paramMap[paramNames[i]] = std::make_shared<Parameter>(1.0f, 1.0f, 0.0f);
			break;
		case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 14: case 15: case 16: case 17:
			paramMap[paramNames[i]] = std::make_shared<Parameter>(10.0f);
			break;
		default:
			break;
		}
	}
}

void Model::reset()
{
	pos.x = 0.0f;
	pos.y = 0.0f;
	scale = 1.0f;
	rotation = 0.0f;
}

void Model::updatePartMap()
{
	partMap.clear();
	meshMap.clear();
	for (int i = 0; i < children.size(); i++)
		updatePartMapRecursive(children[i]);
}

void Model::renderMeshVertice(const std::string &meshName)
{
	if (partMap.find(meshName) == partMap.end())
		return;

	partMap[meshName]->updateVertexData();

	switch (partMap[meshName]->type)
	{
	case ModelPart::PartType::mesh:
		glLineWidth(static_cast<GLfloat>(Settings::meshLineWidth));
		shader.setVec3("uiColor", Settings::meshLineColor);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);

		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointBorderSize * 2 + Settings::meshPointSize));
		shader.setVec3("uiColor", Settings::meshPointBorderColor);
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		shader.setVec3("uiColor", Settings::meshPointColor);
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		break;
	case ModelPart::PartType::warpDeformer:
		glLineWidth(static_cast<GLfloat>(Settings::meshLineWidth));
		shader.setVec3("uiColor", Settings::warpDeformerColor);
		glDrawElements(GL_LINES, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);

		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		break;
	case ModelPart::PartType::rotationDeformer:
		glLineWidth(static_cast<GLfloat>(Settings::rotationDeformerWidth));
		shader.setVec3("uiColor", Settings::rotationDeformerColor);
		glDrawElements(GL_LINES, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);

		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointBorderSize * 2 + Settings::meshPointSize));
		shader.setVec3("uiColor", Settings::meshPointBorderColor);
		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		shader.setVec3("uiColor", Settings::rotationDeformerColor);
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		break;
	default:
		break;
	}
}

void Model::renderHighlightedMesh()
{
	glLineWidth(static_cast<GLfloat>(Settings::meshLineWidth));

	modelMeshes[mouseHoveredID]->updateVertexData();
	shader.setVec3("uiColor", Settings::meshHighlightColor);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelMeshes[mouseHoveredID]->indices.size()), GL_UNSIGNED_INT, 0);

	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
	glDrawElements(GL_POINTS, static_cast<GLsizei>(modelMeshes[mouseHoveredID]->indices.size()), GL_UNSIGNED_INT, 0);
}

void Model::renderSelectedVertices()
{
	shader.setVec3("uiColor", Settings::meshPointSelectedColor);
	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));

	glBindVertexArray(vao);

	//might group all the vertices into a single draw call if it becomes slow, just testing for now
	for (int i = 0; i < selectedVertices.size(); i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &partMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
	}
}

void Model::moveSelectedVertices(const glm::vec2 originalMouseCoord)
{
	ImVec2 mouseCoord = ImGui::GetMousePos();

	glm::vec4 mouseToScreen = glm::inverse(Camera2D::projection) * glm::vec4(mouseCoord.x * 2.0f / Window::width - 1.0f, mouseCoord.y * -2.0f / Window::height + 1.0f, 0.0f, 1.0f);

	for (int i = 0; i < selectedVertices.size(); i++)
	{
		if (partMap[selectedVertices[i].partName]->parent->type == ModelPart::PartType::warpDeformer)
		{
			auto p = std::dynamic_pointer_cast<WarpDeformer>(partMap[selectedVertices[i].partName]->parent);
			
			if (partMap[selectedVertices[i].partName]->type == ModelPart::PartType::rotationDeformer)
			{
				//root selected
				if (selectedVertices[i].index == 0)
				{
					glm::vec2 unwarpedPoint = p->unwarpPoint(glm::inverse(p->transform) * glm::vec4(glm::vec2(mouseToScreen) - originalMouseCoord + initialVerticesPos[&selectedVertices[i]], 0.0f, 1.0f));
					glm::mat4 tempMat = glm::mat4(1.0f);
					tempMat = glm::translate(tempMat, glm::vec3(initialVerticesPos[&selectedVertices[i]], 0.0f));
					tempMat = glm::rotate(tempMat, glm::radians(partMap[selectedVertices[i].partName]->rotation), glm::vec3(0.0f, 0.0f, 1.0f));
					tempMat = glm::scale(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));

					glm::mat4 tempMat2 = glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(partMap[selectedVertices[i].partName]->rotation), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));
					partMap[selectedVertices[i].partName]->pos = tempMat2 * glm::inverse(tempMat) * glm::vec4(unwarpedPoint, 0.0f, 1.0f) + glm::vec4(initialVerticesPos[&selectedVertices[i]], 0.0f, 0.0f);

					//keep children's global position
					if (Event::keyDown(GLFW_KEY_LEFT_ALT))
						for (int i = 0; i < partMap[selectedVertices[i].partName]->children.size(); i++)
						{

						}
				}
				//end selected
				else
				{
					glm::mat4 tempMat = glm::mat4(1.0f);
					tempMat = glm::translate(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->pos.x, partMap[selectedVertices[i].partName]->pos.y, 0.0f));
					tempMat = glm::rotate(tempMat, glm::radians(initialVerticesPos[&selectedVertices[i]].x), glm::vec3(0.0f, 0.0f, 1.0f));
					tempMat = glm::scale(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));

					glm::vec2 mousePos = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * mouseToScreen;
					glm::vec2 mousePosOriginal = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * glm::vec4(partMap[selectedVertices[i].partName]->vertices[0].position, 0.0f, 1.0f);

					partMap[selectedVertices[i].partName]->rotation = glm::degrees(std::atan2((mousePos.y - mousePosOriginal.y), (mousePos.x - mousePosOriginal.x))) + initialVerticesPos[&selectedVertices[i]].x - 90.0f;
				}
			}
			else
			{
				glm::vec2 unwarpedPoint = p->unwarpPoint(glm::inverse(p->transform) * glm::vec4(glm::vec2(mouseToScreen) - originalMouseCoord + initialVerticesPos[&selectedVertices[i]], 0.0f, 1.0f));

				partMap[selectedVertices[i].partName]->localVertexPositions[selectedVertices[i].index] = glm::vec2(glm::inverse(partMap[selectedVertices[i].partName]->localTransform) * glm::vec4(unwarpedPoint, 0.0f, 1.0f));
			}
		}
		else
		{
			glm::vec2 mousePos = glm::inverse(partMap[selectedVertices[i].partName]->transform) * mouseToScreen;
			glm::vec2 mousePosOriginal = glm::inverse(partMap[selectedVertices[i].partName]->transform) * glm::vec4(originalMouseCoord, 0.0f, 1.0f);

			if (partMap[selectedVertices[i].partName]->type == ModelPart::PartType::rotationDeformer)
			{
				//root selected
				if (selectedVertices[i].index == 0)
				{
					partMap[selectedVertices[i].partName]->pos = glm::vec2(glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(partMap[selectedVertices[i].partName]->rotation), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f)) * glm::vec4(mousePos - mousePosOriginal, 0.0f, 1.0f)) + initialVerticesPos[&selectedVertices[i]];

					//keep children's global position
					if (Event::keyDown(GLFW_KEY_LEFT_ALT))
						for (int i = 0; i < partMap[selectedVertices[i].partName]->children.size(); i++)
						{
							//partMap[selectedVertices[i].partName]->children[i]->pos = 
						}
				}
				//end selected
				else
				{
					glm::mat4 tempMat = glm::mat4(1.0f);
					tempMat = glm::translate(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->pos.x, partMap[selectedVertices[i].partName]->pos.y, 0.0f));
					tempMat = glm::rotate(tempMat, glm::radians(initialVerticesPos[&selectedVertices[i]].x), glm::vec3(0.0f, 0.0f, 1.0f));
					tempMat = glm::scale(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));

					mousePos = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * mouseToScreen;
					mousePosOriginal = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * glm::vec4(partMap[selectedVertices[i].partName]->vertices[0].position, 0.0f, 1.0f);

					partMap[selectedVertices[i].partName]->rotation = glm::degrees(std::atan2((mousePos.y - mousePosOriginal.y), (mousePos.x - mousePosOriginal.x))) + initialVerticesPos[&selectedVertices[i]].x - 90.0f;
				}
			}
			else
				partMap[selectedVertices[i].partName]->localVertexPositions[selectedVertices[i].index] = mousePos - mousePosOriginal + initialVerticesPos[&selectedVertices[i]];
		}
	}
}

void Model::updateOriginalVertexPositions()
{
	initialVerticesPos.clear();
	for (int i = 0; i < selectedVertices.size(); i++)
	{
		//if rotation deformer, save other data
		if (partMap[selectedVertices[i].partName]->type == ModelPart::PartType::rotationDeformer)
		{
			if (selectedVertices[i].index == 0)
				if (partMap[selectedVertices[i].partName]->parent->type == ModelPart::PartType::warpDeformer)
					initialVerticesPos[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->vertices[0].position;
				else
					initialVerticesPos[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->pos;
			else
				initialVerticesPos[&selectedVertices[i]] = glm::vec2(partMap[selectedVertices[i].partName]->rotation, 0.0f);
		}
		else
		{
			if (partMap[selectedVertices[i].partName]->parent->type == ModelPart::PartType::warpDeformer)
				initialVerticesPos[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index].position;
			else
				initialVerticesPos[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->localVertexPositions[selectedVertices[i].index];
		}
	}
}

void Model::renderClosestVertex(const std::string& partName, int vertexIndex)
{
	shader.setVec3("uiColor", Settings::meshPointHighlightColor);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &partMap[partName]->vertices[vertexIndex], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
}

void Model::generateTestBoxMesh(std::string partName, int boxSizeX, int boxSizeY)
{
	for (int i = 0; i < modelMeshes.size(); i++)
	{
		if (modelMeshes[i]->name == partName)
		{
			modelMeshes[i]->createTriMesh(boxSizeX, boxSizeY, atlasWidth, atlasHeight);
		}
	}
}

bool Model::checkNameExists(const std::string& name)
{
	return partMap.find(name) != partMap.end();
}

bool Model::checkSameParent(const std::vector<std::string>& selectedParts)
{
	std::shared_ptr<ModelPart> commonParent;
	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		else if (!commonParent)
			commonParent = partMap[selectedParts[i]]->parent;
		else if(commonParent != partMap[selectedParts[i]]->parent)
			return false;
	}
	return true;
}

//add check if folder was selected too
void Model::addWarpDeformer(std::string name, const std::vector<std::string>& selectedParts, int countX, int countY)
{
	std::shared_ptr<ModelPart> parentPart;
	std::shared_ptr<ModelPart> newDeformer;

	//find parent
	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) != partMap.end())
		{
			parentPart = partMap[selectedParts[i]]->parent;
			break;
		}
	}

	if (!parentPart)
		return;

	//figure out box size
	float top = -FLT_MAX, bottom = FLT_MAX, left = FLT_MAX, right = -FLT_MAX;

	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		for (int j = 0; j < partMap[selectedParts[i]]->vertices.size(); j++)
		{
			if (partMap[selectedParts[i]]->vertices[j].position.y > top)
				top = partMap[selectedParts[i]]->vertices[j].position.y;
			if (partMap[selectedParts[i]]->vertices[j].position.y < bottom)
				bottom = partMap[selectedParts[i]]->vertices[j].position.y;
			if (partMap[selectedParts[i]]->vertices[j].position.x < left)
				left = partMap[selectedParts[i]]->vertices[j].position.x;
			if (partMap[selectedParts[i]]->vertices[j].position.x > right)
				right = partMap[selectedParts[i]]->vertices[j].position.x;
		}
	}
	glm::vec2 localCoord = glm::inverse(parentPart->transform) * glm::vec4(left + (right - left) / 2.0f, bottom + (top - bottom) / 2.0f, 0.0f, 1.0f);

	parentPart->children.push_back(std::make_shared<WarpDeformer>(name, countX, countY, localCoord.x, localCoord.y, right - left, top - bottom));

	newDeformer = parentPart->children.back();
	newDeformer->parent = parentPart;

	//move parts to new parent
	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		newDeformer->children.push_back(partMap[selectedParts[i]]);

		partMap[selectedParts[i]]->parent->children.erase(
			remove(partMap[selectedParts[i]]->parent->children.begin(), partMap[selectedParts[i]]->parent->children.end(), partMap[selectedParts[i]]),
			partMap[selectedParts[i]]->parent->children.end());
	}

	for (int i = 0; i < newDeformer->children.size(); i++)
	{
		newDeformer->children[i]->parent = newDeformer;
		newDeformer->children[i]->pos -= glm::vec2(newDeformer->pos.x, newDeformer->pos.y);
	}

	partMap[name] = newDeformer;
}

void Model::addRotationDeformer(std::string name, const std::vector<std::string>& selectedParts)
{
	std::shared_ptr<ModelPart> parentPart;
	std::shared_ptr<ModelPart> newDeformer;

	//find parent
	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) != partMap.end())
		{
			parentPart = partMap[selectedParts[i]]->parent;
			break;
		}
	}

	if (!parentPart)
		return;

	//figure out box size
	float top = -FLT_MAX, bottom = FLT_MAX, left = FLT_MAX, right = -FLT_MAX;

	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		for (int j = 0; j < partMap[selectedParts[i]]->vertices.size(); j++)
		{
			if (partMap[selectedParts[i]]->vertices[j].position.y > top)
				top = partMap[selectedParts[i]]->vertices[j].position.y;
			if (partMap[selectedParts[i]]->vertices[j].position.y < bottom)
				bottom = partMap[selectedParts[i]]->vertices[j].position.y;
			if (partMap[selectedParts[i]]->vertices[j].position.x < left)
				left = partMap[selectedParts[i]]->vertices[j].position.x;
			if (partMap[selectedParts[i]]->vertices[j].position.x > right)
				right = partMap[selectedParts[i]]->vertices[j].position.x;
		}
	}
	glm::vec2 localCoord = glm::inverse(parentPart->transform) * glm::vec4(left + (right - left) / 2.0f, bottom + (top - bottom) / 2.0f, 0.0f, 1.0f);

	parentPart->children.push_back(std::make_shared<RotationDeformer>(name, localCoord.x, localCoord.y));

	newDeformer = parentPart->children.back();
	newDeformer->parent = parentPart;

	//move parts to new parent
	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		newDeformer->children.push_back(partMap[selectedParts[i]]);

		partMap[selectedParts[i]]->parent->children.erase(
			remove(partMap[selectedParts[i]]->parent->children.begin(), partMap[selectedParts[i]]->parent->children.end(), partMap[selectedParts[i]]),
			partMap[selectedParts[i]]->parent->children.end());
	}

	for (int i = 0; i < newDeformer->children.size(); i++)
	{
		newDeformer->children[i]->parent = newDeformer;
		newDeformer->children[i]->pos -= glm::vec2(newDeformer->pos.x, newDeformer->pos.y);
	}

	partMap[name] = newDeformer;
}

void Model::resetParams()
{
	for (int i = 0; i < paramNames.size(); i++)
	{
		paramValues[paramNames[i]] = paramMap[paramNames[i]]->defaultValue;
	}
}

void Model::addKeyform(const std::string& partName, const std::string& paramName, float keyvalue)
{
	//might come up with system to deal with duplicate/deleted keyforms
	//also copying values when adding keyforms to parts that already have keyforms

	//add param to model
	paramMap[paramName]->keyValues.push_back(keyvalue);
	std::sort(paramMap[paramName]->keyValues.begin(), paramMap[paramName]->keyValues.end());

	//add param to part
	int paramNameIndex;
	auto it = std::find(partMap[partName]->paramNames.begin(), partMap[partName]->paramNames.end(), paramName);
	//if first keyform for parameter name
	if (it == partMap[partName]->paramNames.end())
	{
		paramNameIndex = static_cast<int>(partMap[partName]->paramNames.size());
		partMap[partName]->paramNames.push_back(paramName);
	}
	else
		paramNameIndex = static_cast<int>(it - partMap[partName]->paramNames.begin());

	partMap[partName]->paramKeyvalues.resize(partMap[partName]->paramNames.size());
	partMap[partName]->paramWeights.resize(partMap[partName]->paramNames.size());
	partMap[partName]->paramKeyvalues[paramNameIndex].push_back(keyvalue);
	std::sort(partMap[partName]->paramKeyvalues[paramNameIndex].begin(), partMap[partName]->paramKeyvalues[paramNameIndex].end());

	partMap[partName]->keyformsPerDimension.resize(partMap[partName]->paramNames.size());
	int keyformInterpolateCount = 1, totalKeyformCount = 1, prevKeyformCount = static_cast<int>(partMap[partName]->keyforms.size());
	for (int i = 0; i < partMap[partName]->paramKeyvalues.size(); i++)
	{
		keyformInterpolateCount *= 2;
		partMap[partName]->keyformsPerDimension[i] = totalKeyformCount;
		totalKeyformCount *= static_cast<int>(partMap[partName]->paramKeyvalues[i].size());
	}
	partMap[partName]->keyformWeights.resize(keyformInterpolateCount);
	partMap[partName]->keyformIndices.resize(keyformInterpolateCount);
	partMap[partName]->keyforms.resize(totalKeyformCount);
}

int Model::findClosestVertex(const std::vector<std::string>& selectedParts, int* partNum)
{
	int closestVertexIndex = -1;

	float closestDistance = FLT_MAX;

	float mouseX = ImGui::GetMousePos().x;
	float mouseY = ImGui::GetMousePos().y;

	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		for (int v = 0; v < partMap[selectedParts[i]]->vertices.size(); v++)
		{
			auto vert = Camera2D::projection * glm::vec4(partMap[selectedParts[i]]->vertices[v].position.x, partMap[selectedParts[i]]->vertices[v].position.y, 0.0f, 1.0f);
			float pixelDistance = static_cast<float>(pow(mouseX - (vert.x + 1.0f) * Window::width / 2, 2) + pow(mouseY - (vert.y - 1.0f) * Window::height / -2, 2));
			if (pixelDistance <= Settings::vertexDetectionDistance * Settings::vertexDetectionDistance && closestDistance > pixelDistance)
			{
				*partNum = i;
				closestVertexIndex = v;
				closestDistance = pixelDistance;
			}
		}
	}

	return closestVertexIndex;
}

void Model::updateCanvasCoord()
{
	canvasCoords[0] = glm::vec2(psdDimension.x / -2.0f, psdDimension.y / 2.0f);
	canvasCoords[1] = glm::vec2(psdDimension.x / 2.0f, psdDimension.y / 2.0f);
	canvasCoords[2] = glm::vec2(psdDimension.x / 2.0f, psdDimension.y / -2.0f);
	canvasCoords[3] = glm::vec2(psdDimension.x / -2.0f, psdDimension.y / -2.0f);

	glBindVertexArray(canvasVao);

	glBindBuffer(GL_ARRAY_BUFFER, canvasVbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), &canvasCoords[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, canvasEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 32, &rectLineIndices[0], GL_STATIC_DRAW);

	//set vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);
}

void Model::showMeshClippingMenu(const std::string& meshName)
{
	ImGui::Separator();

	ImGui::Text("Clipping");
	ImGui::Text("Clipped: %d", meshMap[meshName]->clipMeshes.size());
	ImGui::Checkbox("Invert", &meshMap[meshName]->invertClip);

	for (int i = static_cast<int>(modelMeshes.size()) - 1; i >= 0; i--)
	{
		//don't include self in list
		if (modelMeshes[i]->name != meshName)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

			bool alreadySelected = alreadySelected = std::find(meshMap[meshName]->clipMeshes.begin(), meshMap[meshName]->clipMeshes.end(), modelMeshes[i]->name) != meshMap[meshName]->clipMeshes.end();
			if (alreadySelected)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ImGui::TreeNodeEx(modelMeshes[i]->name.c_str(), nodeFlags);
			if (ImGui::IsItemClicked())
			{
				if (alreadySelected)
					meshMap[meshName]->clipMeshes.erase(remove(meshMap[meshName]->clipMeshes.begin(), meshMap[meshName]->clipMeshes.end(), modelMeshes[i]->name), meshMap[meshName]->clipMeshes.end());
				else
					meshMap[meshName]->clipMeshes.push_back(modelMeshes[i]->name);
			}
		}
	}
}

void Model::bindUniformTextures()
{
	shader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, modelTexColorBuffer);
}

void Model::updatePartMapRecursive(std::shared_ptr<ModelPart> part)
{
	partMap[part->name] = part;

	//also update mesh map
	if (part->type == ModelPart::PartType::mesh)
		meshMap[part->name] = std::dynamic_pointer_cast<ModelMesh>(part);

	for (int i = 0; i < part->children.size(); i++)
	{
		updatePartMapRecursive(part->children[i]);
	}
}

void Model::updateFrameBufferSize()
{
	glBindFramebuffer(GL_FRAMEBUFFER, modelFbo);
	glBindTexture(GL_TEXTURE_2D, modelTexColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Window::width, Window::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexColorBuffer, 0);

	//mouse picking
	glBindTexture(GL_TEXTURE_2D, mousePickBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, Window::width, Window::height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mousePickBuffer, 0);

	glDrawBuffers(2, bufs);

	//might not need rbo
	//glBindRenderbuffer(GL_RENDERBUFFER, modelRbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, Window::width, Window::height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, modelRbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Log::logError("ERROR::FRAMEBUFFER:: Model framebuffer is not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	bindUniformTextures();
}

void Model::update()
{
	shader.setMat4("projection", Camera2D::projection);

	if (Event::windowResized)
		updateFrameBufferSize();

	if (Event::keyDown(GLFW_KEY_R))
		reset();

	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0.0f));
	transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->updateTransform(paramValues);
		children[i]->modelUpdate(paramValues);
	}

	renderOrderMap.clear();
	for (int i = 0; i < modelMeshes.size(); i++)
	{
		renderOrderMap.insert(std::pair<int, int>(modelMeshes[i]->renderOrder, i));
	}
}

void Model::renderMaskedMesh(int meshNum)
{
	//draw parent mesh's alpha
	shader.setInt("mode", 9);
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	for (int j = 0; j < modelMeshes[meshNum]->clipMeshes.size(); j++)
	{
		shader.setVec4("texColor", meshMap[modelMeshes[meshNum]->clipMeshes[j]]->color);
		meshMap[modelMeshes[meshNum]->clipMeshes[j]]->render();
	}

	//render based on parent's alpha and own alpha
	shader.setVec4("texColor", modelMeshes[meshNum]->color);

	if (modelMeshes[meshNum]->invertClip)
	{
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
		modelMeshes[meshNum]->render();
		shader.setInt("mode", 6);
		glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ZERO, GL_ZERO);
	}
	else
	{
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_SRC_ALPHA);
		modelMeshes[meshNum]->render();
		shader.setInt("mode", 6);
		glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ZERO, GL_ZERO);
	}
	modelMeshes[meshNum]->render();

	//reset alpha
	shader.setInt("mode", 0);
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ZERO);
	for (int j = 0; j < modelMeshes[meshNum]->clipMeshes.size(); j++)
		meshMap[modelMeshes[meshNum]->clipMeshes[j]]->render();
}

void Model::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, modelFbo);

	//clear framebuffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//might set this for each mesh's setting
	shader.setInt("mode", 0);

	//render each mesh
	for (auto const &[meshRenderOrder, meshIndex] : renderOrderMap)
	{
		if (modelMeshes[meshIndex]->visible)
		{
			//if being clipped others
			if (modelMeshes[meshIndex]->clipMeshes.size())
				renderMaskedMesh(meshIndex);
			//if not being clipped by others
			else
			{
				shader.setVec4("texColor", modelMeshes[meshIndex]->color);

				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
				modelMeshes[meshIndex]->render();
			}
		}
	}

	//fix alpha
	shader.setInt("mode", 10);
	for (auto const& [meshRenderOrder, meshIndex] : renderOrderMap)
	{
		shader.setInt("ID", meshIndex + 1);

		if (modelMeshes[meshIndex]->visible)
		{
			shader.setVec4("texColor", modelMeshes[meshIndex]->color);
			if (!modelMeshes[meshIndex]->clipMeshes.size() || modelMeshes[meshIndex]->invertClip)
				glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			else
				glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ONE);
			modelMeshes[meshIndex]->render();
		}
	}

	//mouse hovering mesh
	if (detectMouseHover)
	{
		detectMouseHover = false;
		ImVec2 mPos = ImGui::GetMousePos();
		mPos.y = Window::height - mPos.y;
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glReadPixels((int)mPos.x, (int)mPos.y, 1, 1, GL_RED_INTEGER, GL_INT, &mouseHoveredID);
		//since buffer cleared to 0 and ID starts at 1
		mouseHoveredID--;
	}
	else
		mouseHoveredID = -1;

	//render from framebuffer to screen
	updateVertexData();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	shader.setMat4("projection", glm::mat4(1.0f));
	shader.setVec4("texColor", glm::vec4(1.0f));

	if (Settings::transparentBackground)
		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	else
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

	int fboRenderMode = 2;
	if (Settings::colorCorrection)
		fboRenderMode = 4;
	if (Settings::effect)
	{
		fboRenderMode++;
		shader.setFloat("timer", static_cast<float>(glfwGetTime()));
	}
	shader.setInt("mode", fboRenderMode);

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

	if (screenshot)
	{
		screenshot = false;

		std::vector<unsigned char> data;
		data.resize(Window::width * Window::height * 4);

		glReadPixels(0, 0, Window::width, Window::height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

		stbi_flip_vertically_on_write(true);
		stbi_write_png("saves/testExports/testFbo.png", Window::width, Window::height, 4, &data[0], Window::width * 4);
	}

	//render the canvas rect
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	if (Settings::showCanvas)
	{
		shader.setMat4("projection", Camera2D::projection);
		shader.setInt("mode", 1);
		glBindVertexArray(canvasVao);
		shader.setVec3("uiColor", Settings::canvasBorderColor);
		glLineWidth(static_cast<GLfloat>(Settings::canvasLineWidth));
		glDrawElements(GL_LINES, static_cast<GLsizei>(8), GL_UNSIGNED_INT, 0);
	}
}
