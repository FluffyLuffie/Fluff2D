#include "Application.h"

Application::Application()
{

}

Application::~Application()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Application::update()
{
	Event::calculateDeltaTime();
	Event::resetKeys();
	glfwPollEvents();

	window.update();
	if (model)
	{
		model->update();
		model->render();

		//testing stuff
		if (selectedParts.size())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			model->shader.setBool("drawPoints", true);

			for (int i = 0; i < selectedParts.size(); i++)
			{
				model->renderMeshVertice(selectedParts[i]);
			}

			Vertex* closestVertex = model->findClosestVertex(selectedParts);
			if (closestVertex)
			{
				//std::cout << "Closest point: " << closestVertex->position.x << " " << closestVertex->position.y << std::endl;
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			model->shader.setBool("drawPoints", false);
		}
	}
	renderCanvas();

	//create imgui frame
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	//docking stuff
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Invis", nullptr, windowFlags);
	ImGui::PopStyleVar(3);
	ImGuiID dockId = ImGui::GetID("Invis");
	ImGui::DockSpace(dockId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	//testing menu
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				fileBrowser.SetTypeFilters({ ".psd", ".ftd" });
				fileBrowser.Open();
			}
			if (ImGui::MenuItem("Open test model"))
				initializeModelFromPsd("saves/tempPsdTest/testModel.psd");
			if(ImGui::MenuItem("Save"))
				saveModel();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Test1");
			ImGui::MenuItem("Test2");
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	fileBrowser.Display();
	if (fileBrowser.HasSelected())
	{
		std::string fileName = fileBrowser.GetSelected().string();
		if (!fileName.compare(fileName.length() - 4, 4, ".psd"))
			initializeModelFromPsd(fileName.c_str());
		//if .ftd file
		else
			Log::logWarning("Implementing reading ftd files later");
		fileBrowser.ClearSelected();
	}

	ImGui::Begin("Toolbar");
	ImGui::End();

	//testing UI
	ImGui::Begin("General");

	ImGui::ColorEdit3("Background", (float*)&Settings::backgroundColor);
	ImGui::SliderInt("Canvas Line Width", &Settings::canvasLineWidth, 1, 20);
	ImGui::ColorEdit3("Canvas Border Color", (float*)&Settings::canvasBorderColor);

	ImGui::SliderInt("Mesh Point Size", &Settings::meshPointSize, 1, 20);
	ImGui::ColorEdit3("Mesh Point Color", (float*)&Settings::meshPointColor);
	ImGui::ColorEdit3("Mesh Point Heighlight Color", (float*)&Settings::meshPointHighlightColor);
	ImGui::ColorEdit3("Mesh Point Selected Color", (float*)&Settings::meshPointSelectedColor);

	ImGui::SliderInt("Mesh Point Border Size", &Settings::meshPointBorderSize, 1, 10);
	ImGui::ColorEdit3("Mesh Point Border Color", (float*)&Settings::meshPointBorderColor);

	ImGui::SliderFloat("Vertex Detection Distance", &Settings::vertexDetectionDistance, 1.0f, 80.0f);

	ImGui::SliderInt("Mesh Line Width", &Settings::meshLineWidth, 1, 20);
	ImGui::ColorEdit3("Mesh Line Color", (float*)&Settings::meshLineColor);

	//might change or delete this later
	if (model)
		ImGui::DragFloat2("Model Position", &model->modelPos.x);

	ImGui::Text("Application average %.3f ms/frame (%d FPS)", 1000.0f / ImGui::GetIO().Framerate, (int)round(ImGui::GetIO().Framerate));

	ImGui::End();

	//testing model parts
	ImGui::Begin("Model Parts");
	if (model)
	{
		for (int i = static_cast<int>(model->layerStructure.size() - 1); i >= 0; i--)
		{
			createModelTree(model->layerStructure[i]);
		}
	}
	ImGui::End();

	//the deformer and mesh structure of the model
	ImGui::Begin("Meshes");
	if (model)
	{
		for (int i = static_cast<int>(model->meshStructure.size() - 1); i >= 0; i--)
		{
			createModelTree(model->meshStructure[i]);
		}
	}
	ImGui::End();

	//details of selected things
	ImGui::Begin("Inspector");
	if (model)
	{
		if (selectedParts.size() == 1 && (model->partMap.find(selectedParts[0]) != model->partMap.end()))
		{
			model->partMap[selectedParts[0]]->renderInspector();
		}
		else
		{
			if (selectedParts.size() > 1)
				ImGui::Text("Selected multiple parts: %d", selectedParts.size());
		}
	}
	ImGui::End();

	//view and edit parameters
	ImGui::Begin("Parameters");
	if (model)
	{
		if (ImGui::Button("Reset Values"))
		{
			Log::logInfo("Resetted param values");
			model->resetParams();
		}
		ImGui::Separator();

		for (int i = 0; i < model->paramNames.size(); i++)
		{
			ImGui::SliderFloat(model->paramNames[i].c_str(), &model->paramMap[model->paramNames[i]]->value, model->paramMap[model->paramNames[i]]->minValue, model->paramMap[model->paramNames[i]]->maxValue);
		}
	}
	ImGui::End();

	//log
	ImGui::Begin("Log");
	ImGui::End();
	Log::draw("Log");

	//ImGui::ShowDemoWindow();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window.getWindow());

	checkRunning();
}

void Application::initializeModelFromPsd(const char* fileName)
{
	double startTime = glfwGetTime();

	model = std::make_shared<Model>();
	TextureLoader::loadPsdFile(fileName, model);
	TextureLoader::loadTexture(&model->textureID, "saves/testExports/textureAtlas.png", &model->atlasWidth, &model->atlasHeight, &model->atlasNrChannels);
	model->updateMeshNumber();
	setCanvasCoords();
	model->generateDefaltParams();

	Log::logInfo("Took %f seconds to load PSD", glfwGetTime() - startTime);
}

void Application::saveModel()
{
	if (model)
		SaveSystem::saveModel(model, version);
}

int Application::loadModel(const char* filePath)
{
	//idk what to do here, this is junk for now don't call
	std::cout << "Don't call loadModel, yet" << std::endl;
	//ModelMesh modelMesh();

	//model->partList.push_back(modelMesh);
	std::cout << "pushed back part" << std::endl;

	return 0;
}

void Application::init()
{
	window.init();
	Event::GLFWWin = window.getWindow();

	glGenVertexArrays(1, (GLuint*)(&canvasVAO));
	glGenBuffers(1, (GLuint*)(&canvasVBO));
	glGenBuffers(1, (GLuint*)(&canvasEBO));

	glfwSetFramebufferSizeCallback(window.getWindow(), Event::framebuffer_size_callback);
	glfwSetScrollCallback(window.getWindow(), Event::scroll_callback);
	glfwSetKeyCallback(window.getWindow(), Event::key_callback);

	stbi_set_flip_vertically_on_load(true);

	//imgui thing
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
	ImGui::StyleColorsDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	Log::clear();
}

void Application::checkRunning()
{
	if (glfwWindowShouldClose(window.getWindow()))
	{
		isRunning = false;
	}
}

void Application::setCanvasCoords()
{
	canvasCoords[0] = glm::vec2(model->psdDimension.x / -2.0f, model->psdDimension.y / 2.0f);
	canvasCoords[1] = glm::vec2(model->psdDimension.x / 2.0f, model->psdDimension.y / 2.0f);
	canvasCoords[2] = glm::vec2(model->psdDimension.x / 2.0f, model->psdDimension.y / -2.0f);
	canvasCoords[3] = glm::vec2(model->psdDimension.x / -2.0f, model->psdDimension.y / -2.0f);
}

void Application::renderCanvas()
{
	if (model)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		model->shader.setBool("drawPoints", true);

		glBindVertexArray(canvasVAO);

		glBindBuffer(GL_ARRAY_BUFFER, canvasVBO);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), &canvasCoords[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, canvasEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 32, &canvasIndices[0], GL_STATIC_DRAW);

		//set vertices
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
		glEnableVertexAttribArray(0);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, glm::vec3(model->modelPos.x, model->modelPos.y, 0.0f));
		model->shader.setMat4("transform", transform);
		model->shader.setVec3("uiColor", Settings::canvasBorderColor);
		glLineWidth(static_cast<GLfloat>(Settings::canvasLineWidth));


		//testing draw points
		glDrawElements(GL_LINES, static_cast<GLsizei>(8), GL_UNSIGNED_INT, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		model->shader.setBool("drawPoints", false);
	}
}

void Application::createModelTree(std::shared_ptr<ModelPartUI> currentPart)
{
	bool alreadySelected = std::find(selectedParts.begin(), selectedParts.end(), currentPart->name) != selectedParts.end();
	ImGuiTreeNodeFlags nodeFlags = modelPartsFlags;
	if (alreadySelected)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	if (currentPart->type == ModelPartUI::PartType::divider)
	{
		bool nodeOpen = ImGui::TreeNodeEx(currentPart->name.c_str(), nodeFlags);
		if (ImGui::IsItemClicked())
		{
			if (!Event::keyDown(GLFW_KEY_LEFT_CONTROL) && !ImGui::IsItemToggledOpen())
			{
				selectedParts.clear();
				alreadySelected = false;
			}
			if (!ImGui::IsItemToggledOpen())
			{
				if (alreadySelected)
					selectedParts.erase(remove(selectedParts.begin(), selectedParts.end(), currentPart->name), selectedParts.end());
				else
					selectedParts.push_back(currentPart->name);
			}
		}
		if (nodeOpen)
		{
			for (int i = static_cast<int>(currentPart->children.size() - 1); i >= 0; i--)
			{
				createModelTree(currentPart->children[i]);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx(currentPart->name.c_str(), nodeFlags);

		if (ImGui::IsItemClicked())
		{
			if (!Event::keyDown(GLFW_KEY_LEFT_CONTROL))
			{
				selectedParts.clear();
				alreadySelected = false;
			}
			if (alreadySelected)
				selectedParts.erase(remove(selectedParts.begin(), selectedParts.end(), currentPart->name), selectedParts.end());
			else
				selectedParts.push_back(currentPart->name);
		}
	}
}
