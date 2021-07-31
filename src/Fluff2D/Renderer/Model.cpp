#include "Model.h"

Model::Model()
{
	shader.setShader("saves/shaders/shader.vs", "saves/shaders/shader.fs");
}

Model::~Model() {}

void Model::updateMeshNumber()
{
	//migth change somewhere
	VAO.resize(modelMeshes.size());
	VBO.resize(modelMeshes.size());
	EBO.resize(modelMeshes.size());
	glGenVertexArrays(static_cast<int>(modelMeshes.size()), (GLuint*)(&VAO[0]));
	glGenBuffers(static_cast<int>(modelMeshes.size()), (GLuint*)(&VBO[0]));
	glGenBuffers(static_cast<int>(modelMeshes.size()), (GLuint*)(&EBO[0]));
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
	modelPos.x = 0.0f;
	modelPos.y = 0.0f;
	scale = 1.0f;
	rotation = 0.0f;
}

void Model::updatePartMap(std::shared_ptr<ModelPart> part)
{
	partMap[part->name] = part;
	for (int i = 0; i < part->children.size(); i++)
	{
		updatePartMap(part->children[i]);
	}
}

void Model::renderMeshVertice(int meshNum)
{
	glBindVertexArray(VAO[meshNum]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[meshNum]);
	glBufferData(GL_ARRAY_BUFFER, modelMeshes[meshNum]->vertices.size() * sizeof(Vertex), &modelMeshes[meshNum]->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[meshNum]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelMeshes[meshNum]->indices.size() * sizeof(unsigned int), &modelMeshes[meshNum]->indices[0], GL_STATIC_DRAW);

	//set vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	shader.setMat4("transform", modelMeshes[meshNum]->transform);

	//testing draw points
	shader.setVec3("uiColor", Settings::meshLineColor);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelMeshes[meshNum]->indices.size()), GL_UNSIGNED_INT, 0);

	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointBorderSize * 2 + Settings::meshPointSize));
	shader.setVec3("uiColor", Settings::meshPointBorderColor);
	glDrawElements(GL_POINTS, static_cast<GLsizei>(modelMeshes[meshNum]->indices.size()), GL_UNSIGNED_INT, 0);
	shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
	shader.setVec3("uiColor", Settings::meshPointColor);
	glDrawElements(GL_POINTS, static_cast<GLsizei>(modelMeshes[meshNum]->indices.size()), GL_UNSIGNED_INT, 0);
}

void Model::renderMeshVertice(const std::string &meshName)
{
	glLineWidth(static_cast<GLfloat>(Settings::meshLineWidth));
	for (int i = 0; i < modelMeshes.size(); i++)
	{
		if (meshName == modelMeshes[i]->name)
		{
			renderMeshVertice(i);
			break;
		}
	}
}

void Model::resetParams()
{
	for (int i = 0; i < paramNames.size(); i++)
	{
		paramMap[paramNames[i]]->value = paramMap[paramNames[i]]->defaultValue;
	}

	//paramMap[paramNames[0]]->value = paramMap[paramNames[0]]->defaultValue;
}

Vertex* Model::findClosestVertex(const std::vector<std::string>& selectedParts)
{
	float closestDistance = FLT_MAX;
	Vertex* closestVertex = nullptr;

	float mouseX = ImGui::GetMousePos().x;
	float mouseY = ImGui::GetMousePos().y;
	//std::cout << mouseX << " " << mouseY << std::endl;

	//vertToPixelX = (vert.x + 1.0f) * Window::windowWidth / 2

	int closestMeshNum = -1;
	int closestVertexNum = 0;

	for (int i = 0; i < modelMeshes.size(); i++)
		for (int j = 0; j < selectedParts.size(); j++)
			if (modelMeshes[i]->name == selectedParts[j])
				for (int v = 0; v < modelMeshes[i]->vertices.size(); v++)
				{
					auto vert = projection * modelMeshes[i]->transform * glm::vec4(modelMeshes[i]->vertices[v].position.x, modelMeshes[i]->vertices[v].position.y, 0.0f, 1.0f);
					float pixelDistance = static_cast<float>(pow(mouseX - (vert.x + 1.0f) * Window::windowWidth / 2, 2) + pow(mouseY - (vert.y - 1.0f) * Window::windowHeight / -2, 2));
					if (pixelDistance <= Settings::vertexDetectionDistance * Settings::vertexDetectionDistance && closestDistance > pixelDistance)
					{
						closestMeshNum = i;
						closestVertexNum = v;
						closestDistance = pixelDistance;
					}
				}

	//render dot on closest vertex
	if (closestMeshNum != -1)
	{
		closestVertex = &modelMeshes[closestMeshNum]->vertices[closestVertexNum];


		glBindVertexArray(VAO[closestMeshNum]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[closestMeshNum]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &modelMeshes[closestMeshNum]->vertices[closestVertexNum], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[closestMeshNum]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int), 0, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		shader.setMat4("transform", modelMeshes[closestMeshNum]->transform);

		shader.setVec3("uiColor", Settings::meshPointHighlightColor);
		shader.setFloat("pointSize", static_cast<float>(Settings::meshPointSize));
		glDrawElements(GL_POINTS, GLsizei(1), GL_UNSIGNED_INT, 0);
	}

	return closestVertex;
}

void Model::update()
{
	float scroll = Event::getScroll();
	if (scroll != 0.0f)
	{
		if (scroll < 0.0f && scale > 0.01f)
			scale *= 0.9f;
		else if (scroll > 0.0f && scale < 50)
			scale *= 1.1f;
	}

	if (Event::keyDown(GLFW_KEY_W))
		modelPos.y -= Event::deltaTime * 500 / scale;
	if (Event::keyDown(GLFW_KEY_A))
		modelPos.x += Event::deltaTime * 500 / scale;
	if (Event::keyDown(GLFW_KEY_S))
		modelPos.y += Event::deltaTime * 500 / scale;
	if (Event::keyDown(GLFW_KEY_D))
		modelPos.x -= Event::deltaTime * 500 / scale;

	//testing rotation
	if (Event::keyDown(GLFW_KEY_Q))
		rotation += 100 * Event::deltaTime;
	if (Event::keyDown(GLFW_KEY_E))
		rotation -= 100 * Event::deltaTime;
	if (Event::keyDown(GLFW_KEY_R))
		reset();

	for (int i = 0; i < modelParts.size(); i++)
	{
		modelParts[i]->transform = glm::mat4(1.0f);
		modelParts[i]->transform = glm::translate(modelParts[i]->transform, glm::vec3(modelPos.x, modelPos.y, 0.0f));
		modelParts[i]->transform = glm::rotate(modelParts[i]->transform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

		modelParts[i]->update();

		modelParts[i]->transform = glm::translate(modelParts[i]->transform, glm::vec3(modelParts[i]->pos.x, modelParts[i]->pos.y, 0.0f));

		//updating part map
		updatePartMap(modelParts[i]);
	}
}

void Model::render()
{
	//might only need to call once when loading texture since it doesn't change
	glBindTexture(GL_TEXTURE_2D, textureID);

	//projection, might change to 0 to width or height instead but for now it works
	projection = glm::mat4(1.0f);
	projection = glm::ortho(Window::windowWidth / -2.0f / scale, Window::windowWidth / 2.0f / scale, Window::windowHeight / -2.0f / scale, Window::windowHeight / 2.0f / scale, -1.0f, 1.0f);
	shader.setMat4("projection", projection);

	//render each mesh
	for (int i = 0; i < modelMeshes.size(); i++)
	{
		glBindVertexArray(VAO[i]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, modelMeshes[i]->vertices.size() * sizeof(Vertex), &modelMeshes[i]->vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelMeshes[i]->indices.size() * sizeof(unsigned int), &modelMeshes[i]->indices[0], GL_STATIC_DRAW);

		//set vertices
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		//set tex coords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glEnableVertexAttribArray(1);

		shader.setMat4("transform", modelMeshes[i]->transform);
		shader.setVec4("texColor", modelMeshes[i]->color);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelMeshes[i]->indices.size()), GL_UNSIGNED_INT, 0);
	}
}
