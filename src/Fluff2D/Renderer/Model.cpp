#include "Model.h"

Model::Model() {}

Model::~Model() {}

void Model::setShader()
{
	shader.setShader("saves/shaders/shader.vs", "saves/shaders/shader.fs");
}

void Model::updateMeshNumber()
{
	std::cout << "meshList size: " << meshList.size() << std::endl;

	//migth change somewhere
	VAO.resize(meshList.size());
	VBO.resize(meshList.size());
	EBO.resize(meshList.size());
	glGenVertexArrays(static_cast<int>(meshList.size()), (GLuint*)(&VAO[0]));
	glGenBuffers(static_cast<int>(meshList.size()), (GLuint*)(&VBO[0]));
	glGenBuffers(static_cast<int>(meshList.size()), (GLuint*)(&EBO[0]));
}

void Model::printPartsList()
{
	std::cout << "Model parts list: " << std::endl;
	rootPart.printList(0);
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

		//std::cout << scale << std::endl;
	}

	if (Event::keyPressed(GLFW_KEY_W))
		modelPos.y -= Event::deltaTime * 500 / scale;
	if (Event::keyPressed(GLFW_KEY_A))
		modelPos.x += Event::deltaTime * 500 / scale;
	if (Event::keyPressed(GLFW_KEY_S))
		modelPos.y += Event::deltaTime * 500 / scale;
	if (Event::keyPressed(GLFW_KEY_D))
		modelPos.x -= Event::deltaTime * 500 / scale;

	if (Event::keyPressed(GLFW_KEY_Q))
		testModelRotation = true;
	if (Event::keyPressed(GLFW_KEY_E))
		testModelRotation = false;
}

void Model::render()
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader.use();

	//projection, might change to 0 to width or height instead but for now it works
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::ortho(Window::windowWidth / -2.0f / scale, Window::windowWidth / 2.0f / scale, Window::windowHeight / -2.0f / scale, Window::windowHeight / 2.0f / scale, -1.0f, 1.0f);
	shader.setMat4("projection", projection);

	//render each mesh
	for (int i = 0; i < meshList.size(); i++)
	{
		glBindVertexArray(VAO[i]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, meshList[i].vertices.size() * sizeof(Vertex), &meshList[i].vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshList[i].indices.size() * sizeof(unsigned int), &meshList[i].indices[0], GL_STATIC_DRAW);

		//set vertices
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		//set tex coords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
		glEnableVertexAttribArray(1);

		//transform stuff
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(modelPos.x, modelPos.y, 0.0f));
		if (testModelRotation)
			transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(meshList[i].pos.x, meshList[i].pos.y, 0.0f));

		shader.setMat4("transform", transform);

		shader.setVec4("texColor", meshList[i].color);

		meshList[i].renderMesh();
	}
}
