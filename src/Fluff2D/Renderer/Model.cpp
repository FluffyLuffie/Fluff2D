#include "Model.h"

Model::Model()
{
	type = ModelPart::PartType::model;

	meshShader.setShader("saves/shaders/meshShader.vs", "saves/shaders/meshShader.fs");
	screenShader.setShader("saves/shaders/screenShader.vs", "saves/shaders/screenShader.fs");
	meshShader.use();

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
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glGenTextures(1, &texColorBuffer);
	updateFrameBufferSize();
}

Model::~Model() 
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	glDeleteVertexArrays(1, &canvasVao);
	glDeleteBuffers(1, &canvasVbo);
	glDeleteBuffers(1, &canvasEbo);
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
	glLineWidth(static_cast<GLfloat>(Settings::meshLineWidth));

	partMap[meshName]->updateVertexData();

	meshShader.setMat4("transform", partMap[meshName]->transform);

	switch (partMap[meshName]->type)
	{
	case ModelPart::PartType::mesh:
		meshShader.setVec3("uiColor", Settings::meshLineColor);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);

		meshShader.setFloat("pointSize", static_cast<float>(Settings::meshPointBorderSize * 2 + Settings::meshPointSize));
		meshShader.setVec3("uiColor", Settings::meshPointBorderColor);
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		meshShader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		meshShader.setVec3("uiColor", Settings::meshPointColor);
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		break;
	case ModelPart::PartType::warpDeformer:
		meshShader.setVec3("uiColor", Settings::warpDeformerColor);
		glDrawElements(GL_LINES, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
		meshShader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		glDrawElements(GL_POINTS, static_cast<GLsizei>(partMap[meshName]->indices.size()), GL_UNSIGNED_INT, 0);
	}
}

void Model::renderSelectedVertices()
{
	meshShader.setVec3("uiColor", Settings::meshPointSelectedColor);
	meshShader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));

	//might group all the vertices into a single draw call if it becomes slow, just testing for now
	for (auto const & [vert, partName]: selectedVertices)
	{
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), vert, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		meshShader.setMat4("transform", partMap[partName]->transform);

		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
	}
}

void Model::moveSelectedVertices(const ImVec2& originalMouseCoord)
{
	ImVec2 mouseCoord = ImGui::GetMousePos();

	for (auto const& [vert, partName] : selectedVertices)
	{
		auto mousePos = glm::inverse(partMap[partName]->transform) * glm::inverse(Camera2D::projection) * glm::vec4(mouseCoord.x * 2.0f / Window::windowWidth - 1.0f, mouseCoord.y * -2.0f / Window::windowHeight + 1.0f, 0.0f, 1.0f);
		auto originalMousePos = glm::inverse(partMap[partName]->transform) * glm::inverse(Camera2D::projection) * glm::vec4(originalMouseCoord.x * 2.0f / Window::windowWidth - 1.0f, originalMouseCoord.y * -2.0f / Window::windowHeight + 1.0f, 0.0f, 1.0f);

		vert->position.x = initialVerticesPos[vert].x + mousePos.x - originalMousePos.x;
		vert->position.y = initialVerticesPos[vert].y + mousePos.y - originalMousePos.y;
	}
}

void Model::updateOriginalVertexPositions()
{
	for (auto const& [vert, partName] : selectedVertices)
	{
		initialVerticesPos[vert] = vert->position;
	}
}

void Model::renderClosestVertex(Vertex* closestVert, const std::string& partName)
{
	meshShader.setVec3("uiColor", Settings::meshPointHighlightColor);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), closestVert, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), 0, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	meshShader.setMat4("transform", partMap[partName]->transform);

	glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
}

void Model::generateTestBoxMesh(std::string partName, int boxSizeX, int boxSizeY)
{
	for (int i = 0; i < modelMeshes.size(); i++)
	{
		if (modelMeshes[i]->name == partName)
		{
			modelMeshes[i]->createBoxMesh(boxSizeX, boxSizeY, atlasWidth, atlasHeight);
		}
	}
}

bool Model::checkNameExists(const std::string& name)
{
	if (partMap.find(name) != partMap.end())
		Log::logError("Name overlap, input something else");
	return partMap.find(name) != partMap.end();
}

bool Model::checkSameParent(const std::vector<std::string>& selectedParts)
{
	for (int i = 0; i < selectedParts.size(); i++)
		if (partMap[selectedParts[i]]->parent != partMap[selectedParts[0]]->parent)
			return false;
	return true;
}

//add check if folder was selected too
void Model::addWarpDeformer(std::string name, const std::vector<std::string>& selectedParts, int countX, int countY)
{
	std::shared_ptr<ModelPart> parentPart= partMap[selectedParts[0]]->parent;
	std::shared_ptr<ModelPart> newDeformer;

	//figure out box size
	float top = -FLT_MAX, bottom = FLT_MAX, left = FLT_MAX, right = -FLT_MAX;
	for (int i = 0; i < selectedParts.size(); i++)
	{
		for (int j = 0; j < partMap[selectedParts[i]]->vertices.size(); j++)
		{
			glm::vec2 worldCoord = partMap[selectedParts[i]]->transform * glm::vec4(partMap[selectedParts[i]]->vertices[j].position.x, partMap[selectedParts[i]]->vertices[j].position.y, 0.0f, 1.0f);

			if (worldCoord.y > top)
				top = worldCoord.y;
			if (worldCoord.y < bottom)
				bottom = worldCoord.y;
			if (worldCoord.x < left)
				left = worldCoord.x;
			if (worldCoord.x > right)
				right = worldCoord.x;
		}
	}


	glm::vec2 localCoord = glm::inverse(parentPart->transform) * glm::vec4(left, bottom, 0.0f, 1.0f);

	parentPart->children.push_back(std::make_shared<WarpDeformer>(name, countX, countY, localCoord.x, localCoord.y, right - left, top - bottom));

	newDeformer = parentPart->children.back();
	newDeformer->parent = parentPart;

	//move parts to new parent
	for (int i = 0; i < selectedParts.size(); i++)
	{
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
	std::shared_ptr<ModelPart> parentPart = partMap[selectedParts[0]]->parent;
	std::shared_ptr<ModelPart> newDeformer;

	parentPart->children.push_back(std::make_shared<RotationDeformer>(name));

	newDeformer = parentPart->children.back();
	newDeformer->parent = parentPart;

	//move parts to new parent
	for (int i = 0; i < selectedParts.size(); i++)
	{
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

Vertex* Model::findClosestVertex(const std::vector<std::string>& selectedParts, int* partNum)
{
	float closestDistance = FLT_MAX;
	Vertex* closestVertex = nullptr;

	float mouseX = ImGui::GetMousePos().x;
	float mouseY = ImGui::GetMousePos().y;

	for (int i = 0; i < selectedParts.size(); i++)
		for (int v = 0; v < partMap[selectedParts[i]]->vertices.size(); v++)
		{
			auto vert = Camera2D::projection * partMap[selectedParts[i]]->transform * glm::vec4(partMap[selectedParts[i]]->vertices[v].position.x, partMap[selectedParts[i]]->vertices[v].position.y, 0.0f, 1.0f);
			float pixelDistance = static_cast<float>(pow(mouseX - (vert.x + 1.0f) * Window::windowWidth / 2, 2) + pow(mouseY - (vert.y - 1.0f) * Window::windowHeight / -2, 2));
			if (pixelDistance <= Settings::vertexDetectionDistance * Settings::vertexDetectionDistance && closestDistance > pixelDistance)
			{
				*partNum = i;
				closestVertex = &partMap[selectedParts[i]]->vertices[v];
				closestDistance = pixelDistance;
			}
		}

	return closestVertex;
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 32, &canvasIndices[0], GL_STATIC_DRAW);

	//set vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);
}

void Model::showMeshMaskingMenu(const std::string& meshName)
{
	ImGui::Separator();

	ImGui::Text("Masking");

	for (int i = static_cast<int>(modelMeshes.size()) - 1; i >= 0; i--)
	{
		//don't include self in list
		if (modelMeshes[i]->name != meshName)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

			bool alreadySelected = alreadySelected = std::find(meshMap[meshName]->maskedMeshes.begin(), meshMap[meshName]->maskedMeshes.end(), modelMeshes[i]->name) != meshMap[meshName]->maskedMeshes.end();
			if (alreadySelected)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ImGui::TreeNodeEx(modelMeshes[i]->name.c_str(), nodeFlags);
			if (ImGui::IsItemClicked())
			{
				if (alreadySelected)
					meshMap[meshName]->maskedMeshes.erase(remove(meshMap[meshName]->maskedMeshes.begin(), meshMap[meshName]->maskedMeshes.end(), modelMeshes[i]->name), meshMap[meshName]->maskedMeshes.end());
				else
					meshMap[meshName]->maskedMeshes.push_back(modelMeshes[i]->name);
			}
		}
	}
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
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Window::windowWidth, Window::windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Window::windowWidth, Window::windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Log::logError("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void Model::prepareMask(int meshNum)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glBlendFuncSeparate(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glColorMask(false, false, false, false);
	glStencilFunc(GL_ALWAYS, 1, 0xff);
	glStencilMask(0xff);

	for (int j = 0; j < modelMeshes[meshNum]->maskedMeshes.size(); j++)
	{
		meshShader.setMat4("transform", meshMap[modelMeshes[meshNum]->maskedMeshes[j]]->transform);
		meshMap[modelMeshes[meshNum]->maskedMeshes[j]]->render();
	}
}

void Model::update()
{
	meshShader.setMat4("projection", Camera2D::projection);

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
}

void Model::render()
{
	//fbo transparency stuff is a bit weird, fix later
	if (useFbo)
	{
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);

		glBindTexture(GL_TEXTURE_2D, textureID);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	//prepare color channels for transparency
	if (Settings::transparentBackground)
	{
		for (int i = 0; i < modelMeshes.size(); i++)
		{
			//stencil buffer masking
			if (modelMeshes[i]->maskedMeshes.size())
				prepareMask(i);

			meshShader.setMat4("transform", modelMeshes[i]->transform);
			meshShader.setVec4("texColor", modelMeshes[i]->color);

			//render masked mesh to screen
			if (modelMeshes[i]->maskedMeshes.size())
			{
				glColorMask(true, true, true, false);
				glStencilFunc(GL_EQUAL, 1, 0xff);
				glStencilMask(0x00);
				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
			}
			else
			{
				glColorMask(true, true, true, false);
				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
			}
			modelMeshes[i]->render();

			glStencilMask(0xff);
			glStencilFunc(GL_ALWAYS, 0, 0xff);
		}
	}

	for (int i = 0; i < modelMeshes.size(); i++)
	{
		//stencil buffer masking
		if (modelMeshes[i]->maskedMeshes.size())
			prepareMask(i);

		meshShader.setMat4("transform", modelMeshes[i]->transform);
		meshShader.setVec4("texColor", modelMeshes[i]->color);

		//render masked mesh to screen
		if (modelMeshes[i]->maskedMeshes.size())
		{
			glColorMask(true, true, true, false);
			glStencilFunc(GL_EQUAL, 1, 0xff);
			glStencilMask(0x00);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
		}
		else
		{
			glColorMask(true, true, true, true);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		modelMeshes[i]->render();

		glStencilMask(0xff);
		glStencilFunc(GL_ALWAYS, 0, 0xff);
	}

	//if the last mesh is masked to something, the alpha mask is turned off so don't forget to turn it back on
	glColorMask(true, true, true, true);

	if (useFbo)
	{
		//render to screen
		screenShader.use();
		updateVertexData();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);

		glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO); 
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

		//return back to using normal shader
		meshShader.use();
	}

	//render the canvas rect
	meshShader.setBool("drawPoints", true);
	glBindVertexArray(canvasVao);
	meshShader.setMat4("transform", transform);
	meshShader.setVec3("uiColor", Settings::canvasBorderColor);
	glLineWidth(static_cast<GLfloat>(Settings::canvasLineWidth));
	glDrawElements(GL_LINES, static_cast<GLsizei>(8), GL_UNSIGNED_INT, 0);
	meshShader.setBool("drawPoints", false);
}
