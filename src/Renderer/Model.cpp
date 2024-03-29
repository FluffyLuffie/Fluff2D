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

	glBindVertexArray(canvasVao);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, (GLuint*)(&selectedVertexVao));
	glGenBuffers(1, (GLuint*)(&selectedVertexVbo));
	glGenBuffers(1, (GLuint*)(&selectedVertexEbo));

	glBindVertexArray(selectedVertexVao);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, (GLuint*)(&closestVertexVao));
	glGenBuffers(1, (GLuint*)(&closestVertexVbo));
	glGenBuffers(1, (GLuint*)(&closestVertexEbo));

	glBindVertexArray(closestVertexVao);
	glEnableVertexAttribArray(0);

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
	glGenFramebuffers(1, &modelTrueFbo);
	glGenTextures(1, &modelTexColorBuffer);
	glGenTextures(1, &modelTexTrueColorBuffer);
	glGenTextures(1, &mousePickBuffer);
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

	glDeleteVertexArrays(1, &selectedVertexVao);
	glDeleteBuffers(1, &selectedVertexVbo);
	glDeleteBuffers(1, &selectedVertexEbo);

	glDeleteVertexArrays(1, &closestVertexVao);
	glDeleteBuffers(1, &closestVertexVbo);
	glDeleteBuffers(1, &closestVertexEbo);

	glDeleteFramebuffers(1, &modelFbo);
	glDeleteFramebuffers(1, &modelTrueFbo);

	glDeleteTextures(1, &modelTexColorBuffer);
	glDeleteTextures(1, &modelTexTrueColorBuffer);
	glDeleteTextures(1, &mousePickBuffer);
}

void Model::generateDefaltParams()
{
	Log::info("Generating default parameters");

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

	resetParams();
}

void Model::reset()
{
	pos.x = 0.0f;
	pos.y = 0.0f;
	rotation = 0.0f;
}

void Model::updatePartMap()
{
	partMap.clear();
	meshMap.clear();
	for (int i = 0; i < children.size(); i++)
		updatePartMapRecursive(children[i]);
}

void Model::renderMeshVertices(const std::string &meshName)
{
	if (partMap.find(meshName) == partMap.end())
		return;

	glBindVertexArray(partMap[meshName]->vao);

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

	glBindVertexArray(modelMeshes[mouseHoveredID]->vao);
	shader.setVec3("uiColor", Settings::meshHighlightColor);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelMeshes[mouseHoveredID]->indices.size()), GL_UNSIGNED_INT, 0);

	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
	glDrawElements(GL_POINTS, static_cast<GLsizei>(modelMeshes[mouseHoveredID]->indices.size()), GL_UNSIGNED_INT, 0);
}

void Model::renderSelectedVertices()
{
	shader.setVec3("uiColor", Settings::meshPointSelectedColor);
	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));

	glBindVertexArray(selectedVertexVao);
	glBindBuffer(GL_ARRAY_BUFFER, selectedVertexVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexSpecifier), (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, selectedVertexEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), rectLineIndices, GL_DYNAMIC_DRAW);

	//might group all the vertices into a single draw call if it becomes slow, just testing for now
	for (int i = 0; i < selectedVertices.size(); i++)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexSpecifier), &partMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index], GL_DYNAMIC_DRAW);
		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
	}
}

void Model::forceRenderVertices(const std::string& meshName)
{
	shader.setVec3("uiColor", Settings::meshPointSelectedColor);
	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointBorderSize * 2 + Settings::meshPointSize));

	glBindVertexArray(selectedVertexVao);
	glBindBuffer(GL_ARRAY_BUFFER, selectedVertexVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, selectedVertexEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), rectLineIndices, GL_DYNAMIC_DRAW);

	//might group all the vertices into a single draw call if it becomes slow, just testing for now
	for (int i = 0; i < meshMap[meshName]->vertices.size(); i++)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2), &meshMap[meshName]->vertices[i].position, GL_DYNAMIC_DRAW);
		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
	}
}

void Model::moveSelectedVertices(const glm::vec2 &originalMouseCoord, int dragMod)
{
	ImVec2 vpDim = ImGui::GetContentRegionAvail();

	glm::vec4 mouseToScreen = glm::inverse(Camera2D::projection) * glm::vec4(Event::viewportMouseCoord.x * 2.0f / vpDim.x - 1.0f, Event::viewportMouseCoord.y * 2.0f / vpDim.y - 1.0f, 0.0f, 1.0f);

	//if shift held, lock to x or y axis
	if (Event::mod & GLFW_MOD_SHIFT)
	{
		if (abs(originalMouseCoord.x - mouseToScreen.x) > abs(originalMouseCoord.y - mouseToScreen.y))
			mouseToScreen.y = originalMouseCoord.y;
		else
			mouseToScreen.x = originalMouseCoord.x;
	}

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
					glm::vec2 unwarpedPoint = p->unwarpPoint(glm::inverse(p->transform) * glm::vec4(glm::vec2(mouseToScreen) - originalMouseCoord + initialDragData[&selectedVertices[i]], 0.0f, 1.0f));
					glm::mat4 tempMat = glm::mat4(1.0f);
					tempMat = glm::translate(tempMat, glm::vec3(initialDragData[&selectedVertices[i]], 0.0f));
					tempMat = glm::rotate(tempMat, glm::radians(partMap[selectedVertices[i].partName]->rotation), glm::vec3(0.0f, 0.0f, 1.0f));
					tempMat = glm::scale(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));

					glm::mat4 tempMat2 = glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(partMap[selectedVertices[i].partName]->rotation), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));
					
					glm::vec2 newPos, oldPos;

					if (partMap[selectedVertices[i].partName]->keyforms.size())
					{
						newPos = glm::vec2(tempMat2 * glm::inverse(tempMat) * glm::vec4(unwarpedPoint, 0.0f, 1.0f)) + initialDragData[&selectedVertices[i]] - partMap[selectedVertices[i].partName]->basePos;
						oldPos = partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaPosition;
						partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaPosition = newPos;
					}
					else
					{
						newPos = glm::vec2(tempMat2 * glm::inverse(tempMat) * glm::vec4(unwarpedPoint, 0.0f, 1.0f)) + initialDragData[&selectedVertices[i]];
						oldPos = partMap[selectedVertices[i].partName]->basePos;
						partMap[selectedVertices[i].partName]->basePos = newPos;
					}

					//a bit off when in warped rect
					if (dragMod & GLFW_MOD_ALT)
					{
						auto r = std::dynamic_pointer_cast<RotationDeformer>(partMap[selectedVertices[i].partName]);

						r->changeCenterPoint(newPos - oldPos);
					}
				}
				//end selected
				else
				{
					glm::mat4 tempMat = glm::mat4(1.0f);
					tempMat = glm::translate(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->pos, 0.0f));
					if (partMap[selectedVertices[i].partName]->keyforms.size())
						tempMat = glm::rotate(tempMat, glm::radians(initialDragData[&selectedVertices[i]].x + partMap[selectedVertices[i].partName]->baseRotation), glm::vec3(0.0f, 0.0f, 1.0f));
					tempMat = glm::scale(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));

					glm::vec2 mousePos = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * mouseToScreen;
					glm::vec2 mousePosOriginal = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * glm::vec4(partMap[selectedVertices[i].partName]->vertices[0].position, 0.0f, 1.0f);

					if (partMap[selectedVertices[i].partName]->keyforms.size())
						partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaRotation = glm::degrees(std::atan2((mousePos.y - mousePosOriginal.y), (mousePos.x - mousePosOriginal.x))) + initialDragData[&selectedVertices[i]].x - 90.0f;
					else
						partMap[selectedVertices[i].partName]->baseRotation = glm::degrees(std::atan2((mousePos.y - mousePosOriginal.y), (mousePos.x - mousePosOriginal.x))) + initialDragData[&selectedVertices[i]].x - 90.0f;
				}
			}
			else
			{
				glm::vec2 unwarpedPoint = p->unwarpPoint(glm::inverse(p->transform) * glm::vec4(glm::vec2(mouseToScreen) - originalMouseCoord + initialDragData[&selectedVertices[i]], 0.0f, 1.0f));

				partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaVertices[selectedVertices[i].index] = glm::vec2(glm::inverse(partMap[selectedVertices[i].partName]->localTransform) * glm::vec4(unwarpedPoint, 0.0f, 1.0f)) - partMap[selectedVertices[i].partName]->originalVertexPositions[selectedVertices[i].index];
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
					glm::vec2 newPos = glm::vec2(glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(partMap[selectedVertices[i].partName]->rotation), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f)) * glm::vec4(mousePos - mousePosOriginal, 0.0f, 1.0f)) + initialDragData[&selectedVertices[i]];
					glm::vec2 oldPos;

					if (partMap[selectedVertices[i].partName]->keyforms.size())
					{
						oldPos = partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaPosition;
						partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaPosition = newPos;
					}
					else
					{
						oldPos = partMap[selectedVertices[i].partName]->basePos;
						partMap[selectedVertices[i].partName]->basePos = newPos;
					}

					//keep children's global position
					if (dragMod & GLFW_MOD_ALT)
					{
						auto r = std::dynamic_pointer_cast<RotationDeformer>(partMap[selectedVertices[i].partName]);
						r->changeCenterPoint(newPos - oldPos);
					}
				}
				//end selected
				else
				{
					glm::mat4 tempMat = glm::mat4(1.0f);
					tempMat = glm::translate(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->pos, 0.0f));
					if (partMap[selectedVertices[i].partName]->keyforms.size())
						tempMat = glm::rotate(tempMat, glm::radians(initialDragData[&selectedVertices[i]].x + partMap[selectedVertices[i].partName]->baseRotation), glm::vec3(0.0f, 0.0f, 1.0f));
					tempMat = glm::scale(tempMat, glm::vec3(partMap[selectedVertices[i].partName]->scale, 1.0f));

					mousePos = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * mouseToScreen;
					mousePosOriginal = glm::inverse(partMap[selectedVertices[i].partName]->parent->transform * tempMat) * glm::vec4(partMap[selectedVertices[i].partName]->vertices[0].position, 0.0f, 1.0f);

					if (partMap[selectedVertices[i].partName]->keyforms.size())
						partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaRotation = glm::degrees(std::atan2((mousePos.y - mousePosOriginal.y), (mousePos.x - mousePosOriginal.x))) + initialDragData[&selectedVertices[i]].x - 90.0f;
					else
						partMap[selectedVertices[i].partName]->baseRotation = glm::degrees(std::atan2((mousePos.y - mousePosOriginal.y), (mousePos.x - mousePosOriginal.x))) + initialDragData[&selectedVertices[i]].x - 90.0f;
				}
			}
			else
				partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaVertices[selectedVertices[i].index] = mousePos - mousePosOriginal + initialDragData[&selectedVertices[i]];
		}
	}
}

void Model::moveMeshVertices(const glm::vec2& originalMouseCoord, int dragMod)
{
	ImVec2 vpDim = ImGui::GetContentRegionAvail();
	glm::vec2 mouseToScreen = glm::inverse(Camera2D::projection) * glm::vec4(Event::viewportMouseCoord.x * 2.0f / vpDim.x - 1.0f, Event::viewportMouseCoord.y * 2.0f / vpDim.y - 1.0f, 0.0f, 1.0f);

	//if shift held, lock to x or y axis
	if (Event::mod & GLFW_MOD_SHIFT)
	{
		if (abs(originalMouseCoord.x - mouseToScreen.x) > abs(originalMouseCoord.y - mouseToScreen.y))
			mouseToScreen.y = originalMouseCoord.y;
		else
			mouseToScreen.x = originalMouseCoord.x;
	}

	glm::vec2 mouseDelta = mouseToScreen - originalMouseCoord;

	for (int i = 0; i < selectedVertices.size(); i++)
	{
		glm::vec2 newPos = initialDragData[&selectedVertices[i]] + mouseDelta;
		meshMap[selectedVertices[i].partName]->originalVertexPositions[selectedVertices[i].index] = newPos;
		meshMap[selectedVertices[i].partName]->localVertexPositions[selectedVertices[i].index] = newPos;
		meshMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index].position = meshMap[selectedVertices[i].partName]->transform * glm::vec4(newPos, 0.0f, 1.0f);
		meshMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index].texCoord = meshMap[selectedVertices[i].partName]->posToTexCoord(newPos, atlasWidth, atlasHeight);
	}
}

void Model::updateOriginalVertexPositions()
{
	initialDragData.clear();
	for (int i = 0; i < selectedVertices.size(); i++)
	{
		//if rotation deformer, save other data
		if (partMap[selectedVertices[i].partName]->type == ModelPart::PartType::rotationDeformer)
		{
			if (partMap[selectedVertices[i].partName]->keyforms.size() && partMap[selectedVertices[i].partName]->keyformIndex == -1)
				continue;

			//if root selected
			if (selectedVertices[i].index == 0)
			{
				if (partMap[selectedVertices[i].partName]->parent->type == ModelPart::PartType::warpDeformer)
					initialDragData[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->vertices[0].position;
				else
				{
					if (partMap[selectedVertices[i].partName]->keyforms.size())
						initialDragData[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaPosition;
					else
						initialDragData[&selectedVertices[i]] = glm::vec2(partMap[selectedVertices[i].partName]->basePos);
				}
			}
			//if end selected
			else
			{
				if (partMap[selectedVertices[i].partName]->keyforms.size())
					initialDragData[&selectedVertices[i]] = glm::vec2(partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaRotation, 0.0f);
				else
					initialDragData[&selectedVertices[i]] = glm::vec2();
			}
		}
		else
		{
			if (partMap[selectedVertices[i].partName]->keyformIndex == -1)
				continue;

			if (partMap[selectedVertices[i].partName]->parent->type == ModelPart::PartType::warpDeformer)
				initialDragData[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->vertices[selectedVertices[i].index].position;
			else
				initialDragData[&selectedVertices[i]] = partMap[selectedVertices[i].partName]->keyforms[partMap[selectedVertices[i].partName]->keyformIndex].deltaVertices[selectedVertices[i].index];
		}
	}
}

void Model::updateOriginalMeshPositions()
{
	initialDragData.clear();
	for (int i = 0; i < selectedVertices.size(); i++)
	{
		initialDragData[&selectedVertices[i]] = meshMap[selectedVertices[0].partName]->originalVertexPositions[selectedVertices[i].index];
	}
}

void Model::renderClosestVertex(const std::string& partName, int vertexIndex)
{
	if (partMap[partName]->vertices.size() > vertexIndex)
	{
		shader.setVec3("uiColor", Settings::meshPointHighlightColor);

		glBindVertexArray(closestVertexVao);

		glBindBuffer(GL_ARRAY_BUFFER, closestVertexVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &partMap[partName]->vertices[vertexIndex], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, closestVertexEbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), rectLineIndices, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
	}
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
		newDeformer->children[i]->basePos -= glm::vec2(newDeformer->pos.x, newDeformer->pos.y);
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
		newDeformer->children[i]->basePos -= glm::vec2(newDeformer->pos.x, newDeformer->pos.y);
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

void Model::calculateModelParam()
{
	//clear
	for (int i = 0; i < paramNames.size(); i++)
		paramMap[paramNames[i]]->keyValues.clear();

	//check all keyvalues from parts
	for (auto const& [partName, part] : partMap)
		for (int i = 0; i < part->paramNames.size(); i++)
			for (int j = 0; j < part->paramKeyvalues[i].size(); j++)
				if (std::find(paramMap[part->paramNames[i]]->keyValues.begin(), paramMap[part->paramNames[i]]->keyValues.end(), part->paramKeyvalues[i][j]) == paramMap[part->paramNames[i]]->keyValues.end())
				{
					paramMap[part->paramNames[i]]->keyValues.push_back(part->paramKeyvalues[i][j]);
					std::sort(paramMap[part->paramNames[i]]->keyValues.begin(), paramMap[part->paramNames[i]]->keyValues.end());
				}
}

int Model::findClosestVertex(const std::vector<std::string>& selectedParts, int* partNum)
{
	int closestVertexIndex = -1;

	float closestDistance = FLT_MAX;

	ImVec2 vpDim = ImGui::GetContentRegionAvail();

	for (int i = 0; i < selectedParts.size(); i++)
	{
		if (partMap.find(selectedParts[i]) == partMap.end())
			continue;
		for (int v = 0; v < partMap[selectedParts[i]]->vertices.size(); v++)
		{
			auto vert = Camera2D::projection * glm::vec4(partMap[selectedParts[i]]->vertices[v].position.x, partMap[selectedParts[i]]->vertices[v].position.y, 0.0f, 1.0f);
			vert.x = (vert.x + 1.0f) * vpDim.x / 2;
			vert.y = (vert.y + 1.0f) * vpDim.y / 2;
			float pixelDistance = static_cast<float>(pow(Event::viewportMouseCoord.x - vert.x, 2) + pow(Event::viewportMouseCoord.y - vert.y, 2));
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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glBindFramebuffer(GL_FRAMEBUFFER, 1);
	glBindTexture(GL_TEXTURE_2D, modelTexTrueColorBuffer);

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

void Model::updateFrameBufferSize(int x, int y)
{
	fbX = x;
	fbY = y;

	glBindFramebuffer(GL_FRAMEBUFFER, modelFbo);
	glBindTexture(GL_TEXTURE_2D, modelTexColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbX, fbY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexColorBuffer, 0);
	glViewport(0, 0, x, y);

	//mouse picking
	glBindTexture(GL_TEXTURE_2D, mousePickBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, fbX, fbY, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mousePickBuffer, 0);
	glViewport(0, 0, x, y);
	glDrawBuffers(2, bufs);
	//read mesh id for default
	glReadBuffer(GL_COLOR_ATTACHMENT1);

	//true color
	glBindFramebuffer(GL_FRAMEBUFFER, modelTrueFbo);
	glBindTexture(GL_TEXTURE_2D, modelTexTrueColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbX, fbY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexTrueColorBuffer, 0);
	glViewport(0, 0, x, y);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Log::error("ERROR::FRAMEBUFFER:: Model framebuffer is not complete!");

	bindUniformTextures();
}

void Model::update()
{
	shader.setMat4("projection", Camera2D::projection);

	if (Event::keyDown(GLFW_KEY_R))
		reset();

	transform = glm::mat4(1.0f);
	transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0.0f));
	transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->updateTransform(paramValues);
		children[i]->modelUpdate(paramValues);
		children[i]->updateVertexData();
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

	//set the alpha to clipping amount
	shader.setVec4("texColor", modelMeshes[meshNum]->color);
	if (modelMeshes[meshNum]->invertClip)
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
	else
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_SRC_ALPHA);
	modelMeshes[meshNum]->render();

	//render the mesh
	switch (modelMeshes[meshNum]->blendMode)
	{
		//add
		case 1:
			glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO);
			modelMeshes[meshNum]->render();
			break;
			//multiply
		case 2:
			shader.setInt("mode", 4);
			glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO);
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			modelMeshes[meshNum]->render();
			glBlendEquation(GL_FUNC_ADD);
			break;
		//normal or if it doesn't exist
		default:
			shader.setInt("mode", 2);
			glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ZERO, GL_ZERO);
			modelMeshes[meshNum]->render();
			break;
	}

	//reset alpha
	shader.setInt("mode", 0);
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ZERO);
	for (int j = 0; j < modelMeshes[meshNum]->clipMeshes.size(); j++)
		meshMap[modelMeshes[meshNum]->clipMeshes[j]]->render();
}

void Model::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, modelFbo);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shader.setInt("mode", 0);

	//render each mesh
	for (auto const &[meshRenderOrder, meshIndex] : renderOrderMap)
	{
		if (modelMeshes[meshIndex]->visible)
		{
			//if clip
			if (modelMeshes[meshIndex]->clipMeshes.size())
				renderMaskedMesh(meshIndex);
			//if not clip
			else
			{
				shader.setVec4("texColor", modelMeshes[meshIndex]->color);
				switch (modelMeshes[meshIndex]->blendMode)
				{
					//add
					case 1:
						glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ZERO);
						break;
					//multiply
					case 2:
						glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
						break;
					//normal or if it doesn't exist
					default:
						glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
						break;
				}
				modelMeshes[meshIndex]->render();
			}
		}
	}

	//mouse picking
	shader.setInt("mode", 3);
	for (auto const& [meshRenderOrder, meshIndex] : renderOrderMap)
	{
		if (modelMeshes[meshIndex]->visible)
		{
			shader.setInt("ID", meshIndex + 1);
			shader.setVec4("texColor", modelMeshes[meshIndex]->color);
			//TODO: fix alpha with invert clip
			if (modelMeshes[meshIndex]->blendMode != 0 || (!modelMeshes[meshIndex]->invertClip && modelMeshes[meshIndex]->clipMeshes.size()))
				glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ONE);
			else
				glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

			modelMeshes[meshIndex]->render();
		}
	}

	//mouse hovering mesh
	if (detectMouseHover)
	{
		glReadPixels((int)Event::viewportMouseCoord.x, (int)Event::viewportMouseCoord.y, 1, 1, GL_RED_INTEGER, GL_INT, &mouseHoveredID);
		//since buffer cleared to 0 and ID starts at 1
		mouseHoveredID--;
	}
	else
		mouseHoveredID = -1;

	//render true colors
	glBindFramebuffer(GL_FRAMEBUFFER, modelTrueFbo);
	glBindVertexArray(vao);
	shader.setInt("mode", 5);
	shader.setMat4("projection", glm::mat4(1.0f));
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	shader.setMat4("projection", Camera2D::projection);

	if (screenshot)
	{
		screenshot = false;

		std::vector<unsigned char> data;
		data.resize(fbX * fbY * 4);

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, fbX, fbY, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		glReadBuffer(GL_COLOR_ATTACHMENT1);

		//stbi_flip_vertically_on_write(false);
		stbi_write_png("saves/testExports/testFbo.png", fbX, fbY, 4, &data[0], fbX * 4);
	}

	//render the canvas rect
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	shader.setInt("mode", 1);
	if (Settings::showFrame)
	{
		glBindVertexArray(canvasVao);
		shader.setVec3("uiColor", Settings::frameBorderColor);
		glLineWidth(static_cast<GLfloat>(Settings::frameLineWidth));
		glDrawElements(GL_LINES, static_cast<GLsizei>(8), GL_UNSIGNED_INT, 0);
	}
}

void Model::renderEditMesh(const std::string& meshName)
{
	glBindFramebuffer(GL_FRAMEBUFFER, modelFbo);

	glBindVertexArray(vao);
	shader.setMat4("projection", glm::mat4(1.0f));
	shader.setVec3("uiColor", Settings::backgroundColor);
	shader.setInt("mode", 1);
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

	shader.setInt("mode", 0);
	shader.setMat4("projection", Camera2D::projection);

	shader.setVec4("texColor", glm::vec4(1.0f));

	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	partMap[meshName]->render();

	//render true colors
	glBindFramebuffer(GL_FRAMEBUFFER, modelTrueFbo);
	glBindVertexArray(vao);
	shader.setInt("mode", 5);
	shader.setMat4("projection", glm::mat4(1.0f));
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	shader.setMat4("projection", Camera2D::projection);

	//render the canvas rect
	glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	shader.setInt("mode", 1);
	if (Settings::showFrame)
	{
		glBindVertexArray(canvasVao);
		shader.setVec3("uiColor", Settings::frameBorderColor);
		glLineWidth(static_cast<GLfloat>(Settings::frameLineWidth));
		glDrawElements(GL_LINES, static_cast<GLsizei>(8), GL_UNSIGNED_INT, 0);
	}
}
