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

	glLineWidth(static_cast<GLfloat>(Settings::meshLineWidth));

	partMap[meshName]->updateVertexData();

	switch (partMap[meshName]->type)
	{
	case ModelPart::PartType::mesh:
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
		shader.setVec3("uiColor", Settings::warpDeformerColor);
		glDrawElements(GL_LINES, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
	}
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

void Model::moveSelectedVertices(const ImVec2& originalMouseCoord)
{
	ImVec2 mouseCoord = ImGui::GetMousePos();

	for (int i = 0; i < selectedVertices.size(); i++)
	{
		auto mousePos = glm::inverse(partMap[selectedVertices[i].partName]->transform) * glm::inverse(Camera2D::projection) * glm::vec4(mouseCoord.x * 2.0f / Window::width - 1.0f, mouseCoord.y * -2.0f / Window::height + 1.0f, 0.0f, 1.0f);
		auto originalMousePos = glm::inverse(Camera2D::projection) * glm::vec4(originalMouseCoord.x * 2.0f / Window::width - 1.0f, originalMouseCoord.y * -2.0f / Window::height + 1.0f, 0.0f, 1.0f);

		partMap[selectedVertices[i].partName]->localVertexPositions[selectedVertices[i].index].x = initialVerticesPos[&selectedVertices[i]].x + mousePos.x - originalMousePos.x;
		partMap[selectedVertices[i].partName]->localVertexPositions[selectedVertices[i].index].y = initialVerticesPos[&selectedVertices[i]].y + mousePos.y - originalMousePos.y;
	}
}

void Model::updateOriginalVertexPositions()
{
	initialVerticesPos.clear();
	for (int i = 0; i < selectedVertices.size(); i++)
	{
		initialVerticesPos[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index].position;
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
	glm::vec2 localCoord;

	if (parentPart->type != ModelPart::PartType::warpDeformer)
	{
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
		localCoord = glm::inverse(parentPart->transform) * glm::vec4(left + (right - left) / 2.0f, bottom + (top - bottom) / 2.0f, 0.0f, 1.0f);
	}
	else
	{
		for (int i = 0; i < selectedParts.size(); i++)
		{
			if (partMap.find(selectedParts[i]) == partMap.end())
				continue;
			for (int j = 0; j < partMap[selectedParts[i]]->prewarpedVertexPositions.size(); j++)
			{
				if (partMap[selectedParts[i]]->prewarpedVertexPositions[j].y > top)
					top = partMap[selectedParts[i]]->prewarpedVertexPositions[j].y;
				if (partMap[selectedParts[i]]->prewarpedVertexPositions[j].y < bottom)
					bottom = partMap[selectedParts[i]]->prewarpedVertexPositions[j].y;
				if (partMap[selectedParts[i]]->prewarpedVertexPositions[j].x < left)
					left = partMap[selectedParts[i]]->prewarpedVertexPositions[j].x;
				if (partMap[selectedParts[i]]->prewarpedVertexPositions[j].x > right)
					right = partMap[selectedParts[i]]->prewarpedVertexPositions[j].x;
			}
		}
		localCoord = glm::vec4(left + (right - left) / 2.0f, bottom + (top - bottom) / 2.0f, 0.0f, 1.0f);
	}

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

	parentPart->children.push_back(std::make_shared<RotationDeformer>(name));

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
	}

	partMap[name] = newDeformer;
}

void Model::resetParams()
{
	for (int i = 0; i < paramNames.size(); i++)
	{
		paramMap[paramNames[i]]->value = paramMap[paramNames[i]]->defaultValue;
	}
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

void Model::showMeshClippingingMenu(const std::string& meshName)
{
	ImGui::Separator();

	ImGui::Text("Clipping");

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Window::width, Window::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexColorBuffer, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, modelRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, Window::width, Window::height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, modelRbo);

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
		children[i]->update();
	}

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->secondUpdate();
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
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	for (int j = 0; j < modelMeshes[meshNum]->clipMeshes.size(); j++)
	{
		shader.setVec4("texColor", meshMap[modelMeshes[meshNum]->clipMeshes[j]]->color);
		meshMap[modelMeshes[meshNum]->clipMeshes[j]]->render();
	}

	//render based on parent's alpha and own alpha
	shader.setVec4("texColor", modelMeshes[meshNum]->color);
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_SRC_ALPHA);
	modelMeshes[meshNum]->render();
	shader.setInt("mode", 6);
	glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ZERO, GL_ZERO);
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

	//might set this for each mesh's setting
	shader.setInt("mode", 0);

	//clear framebuffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

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
	for (auto const& [meshRenderOrder, meshIndex] : renderOrderMap)
	{
		if (!modelMeshes[meshIndex]->clipMeshes.size() && modelMeshes[meshIndex]->visible)
		{
			shader.setVec4("texColor", modelMeshes[meshIndex]->color);

			glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			modelMeshes[meshIndex]->render();
		}
	}

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
