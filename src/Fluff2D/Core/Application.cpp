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
	Event::update();
	glfwPollEvents();

	window.update();
	Camera2D::update();

	if (Event::keyPressed(GLFW_KEY_ESCAPE))
		drawMenu ^= true;

	//create imgui frame
	if (drawMenu)
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		drawImGui();
	}

	if (model)
	{
		model->update();

		Vertex* closestVertex = nullptr;
		int selectedPartNum = -1;
		if (!Event::io->WantCaptureMouse)
		{
			if (selectedParts.size())
			{
				//testing stuff, move somewhere else later
				closestVertex = model->findClosestVertex(selectedParts, &selectedPartNum);
				if (closestVertex)
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						oldMouseCoord = ImGui::GetMousePos();
						//if ctrl key pressed, add to selected vertices
						if (GLFW_MOD_CONTROL == Event::mod)
						{
							model->selectedVertices[closestVertex] = selectedParts[selectedPartNum];
							model->initialVerticesPos[closestVertex] = closestVertex->position;
						}
						else
						{
							//if clicked vertex is not in selected vertices, clear
							if (model->selectedVertices.find(closestVertex) == model->selectedVertices.end())
							{
								model->selectedVertices.clear();
								model->initialVerticesPos.clear();
								model->selectedVertices[closestVertex] = selectedParts[selectedPartNum];
								model->initialVerticesPos[closestVertex] = closestVertex->position;
							}
						}
					}
				}
				//if clicked on nothing, clear
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					model->selectedVertices.clear();
					model->initialVerticesPos.clear();
				}
			}

			//move vertices
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && GLFW_MOD_CONTROL != Event::mod)
			{
				if (model->selectedVertices.size())
				{
					model->moveSelectedVertices(oldMouseCoord);
					draggingVertices = true;
				}
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && draggingVertices)
			{
				model->updateOriginalVertexPositions();
				draggingVertices = false;
			}
		}

		model->render();

		//testing stuff
		if (selectedParts.size())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			model->meshShader.setBool("drawPoints", true);

			for (int i = 0; i < selectedParts.size(); i++)
			{
				model->renderMeshVertice(selectedParts[i]);
			}

			model->renderSelectedVertices();

			if (closestVertex)
				model->renderClosestVertex(closestVertex, selectedParts[selectedPartNum]);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			model->meshShader.setBool("drawPoints", false);
		}
	}

	//end ImGui frame
	if (drawMenu)
	{
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//load new font
		if (queueFontChange)
		{
			queueFontChange = false;
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->Clear();
			io.Fonts->AddFontFromFileTTF(Settings::fontFile.c_str(), static_cast<float>(Settings::fontSize), NULL, io.Fonts->GetGlyphRangesJapanese());
			ImGui_ImplOpenGL3_CreateFontsTexture();
		}
	}

	glfwSwapBuffers(window.getWindow());

	checkRunning();
}

void Application::initializeModelFromPsd(const char* fileName)
{
	double startTime = glfwGetTime();

	model = std::make_shared<Model>();
	TextureLoader::loadPsdFile(fileName, model);
	TextureLoader::loadTexture(&model->textureID, "saves/testExports/textureAtlas.png", &model->atlasWidth, &model->atlasHeight, &model->atlasNrChannels);
	model->generateDefaltParams();
	model->updatePartMap();

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

	glfwSetFramebufferSizeCallback(window.getWindow(), Event::framebuffer_size_callback);
	glfwSetScrollCallback(window.getWindow(), Event::scroll_callback);
	glfwSetKeyCallback(window.getWindow(), Event::key_callback);

	stbi_set_flip_vertically_on_load(true);

	//imgui thing
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
	ImGui::StyleColorsDark();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.Fonts->AddFontFromFileTTF(Settings::fontFile.c_str(), static_cast<float>(Settings::fontSize), NULL, io.Fonts->GetGlyphRangesJapanese());
	Event::io = &io;

	Log::clear();

	SaveSystem::loadSettings();
}

void Application::checkRunning()
{
	if (glfwWindowShouldClose(window.getWindow()))
	{
		isRunning = false;
	}
}

void Application::createModelTree(std::shared_ptr<ModelPartUI> currentPart)
{
	bool alreadySelected = std::find(selectedParts.begin(), selectedParts.end(), currentPart->name) != selectedParts.end();
	bool nodeOpen = false;
	ImGuiTreeNodeFlags nodeFlags = modelPartsFlags;
	if (alreadySelected)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	switch (currentPart->type)
	{
	case ModelPartUI::PartType::image:
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

			//might implement some way to keep selected vertices if mesh is still selected
			model->selectedVertices.clear();
			model->initialVerticesPos.clear();
			break;
	//for now, treat all of them the same
	case ModelPartUI::PartType::divider:
	case ModelPartUI::PartType::warpDeformer:
	case ModelPartUI::PartType::rotationDeformer:
		nodeOpen = ImGui::TreeNodeEx(currentPart->name.c_str(), nodeFlags);
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
		break;
	default:
		break;
		}
	}
}

void Application::createModelTree(std::shared_ptr<ModelPart> currentPart)
{
	bool alreadySelected = std::find(selectedParts.begin(), selectedParts.end(), currentPart->name) != selectedParts.end();
	bool nodeOpen = false;
	ImGuiTreeNodeFlags nodeFlags = modelPartsFlags;
	if (alreadySelected)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	if (currentPart->type == ModelPart::PartType::mesh)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx(("[M] " + currentPart->name).c_str(), nodeFlags);

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

			//might implement some way to keep selected vertices if mesh is still selected
			model->selectedVertices.clear();
			model->initialVerticesPos.clear();
		}
	}
	else
	{
		if (currentPart->type == ModelPart::PartType::warpDeformer)
			nodeOpen = ImGui::TreeNodeEx(("[W] " + currentPart->name).c_str(), nodeFlags);
		else
			nodeOpen = ImGui::TreeNodeEx(("[R] " + currentPart->name).c_str(), nodeFlags);

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
}

void Application::drawImGui()
{
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
			if (ImGui::MenuItem("Save"))
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

	if (ImGui::Begin("Toolbar"))
	{
		bool meshGeneratorValid = true;
		for (int i = 0; i < selectedParts.size(); i++)
		{
			if (model->partMap[selectedParts[i]]->type != ModelPart::PartType::mesh)
				meshGeneratorValid = false;
		}

		static int boxCount[2] = { 5, 5 };
		if (ImGui::Button("Test Box Vertices") && meshGeneratorValid && model)
		{
			ImGui::OpenPopup("Generate Box Vertices");
		}

		if (ImGui::BeginPopupModal("Generate Box Vertices", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Box Count (X, Y)");
			ImGui::SameLine();
			ImGui::InputInt2("", boxCount);

			if (ImGui::Button("Generate"))
			{
				for (int i = 0; i < selectedParts.size(); i++)
				{
					if (model->partMap[selectedParts[i]]->type == ModelPart::PartType::mesh)
					{
						model->generateTestBoxMesh(selectedParts[i], boxCount[0], boxCount[1]);
					}
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
	ImGui::End();

	//log
	ImGui::Begin("Log");
	ImGui::End();

	//general
	if (ImGui::Begin("General"))
	{
		int selected = static_cast<int>(Settings::selectedLanguage);
		//***THIS DOES ABSOLUTELY NOTHING RIGHT NOW***
		//hard coded in language names, not possible to turn enum names into strings with C++
		//for now, no need to load new font since Meiryo has both English and Japanese
		//might need to change language and load new font after rendering to not mess stuff up
		//i can't figure out how to change language at runtime by reading a file, if you want you can hard code in all the menu dialogues
		if (ImGui::Combo("Language", &selected, u8"English\0日本語"))
		{
			Settings::selectedLanguage = static_cast<Localization::Language>(selected);
				Localization::loadLanguage(Settings::selectedLanguage);
		}
		ImGui::SameLine();
		ImGui::HelpMarker("***THIS DOES ABSOLUTELY NOTHING RIGHT NOW***\nI can't figure out how to support multilingual stuff, only English for now.");
		ImGui::Separator();

		ImGui::SliderInt("Font Size", &Settings::fontSize, 5, 100);
		if (ImGui::Button("Update Font"))
			queueFontChange = true;
		ImGui::Separator();

		if (model)
		{
			if (ImGui::Checkbox("Use FBO", &Settings::useFbo) && !Settings::useFbo)
				glBindTexture(GL_TEXTURE_2D, model->textureID);
			if (Settings::useFbo)
			{
				ImGui::Checkbox("Color Correction", &Settings::colorCorrection);
				ImGui::SameLine();
				ImGui::HelpMarker("It looks weird in this window with transparent background,\nbut fixes itself in the recording software.");
				ImGui::Checkbox("Effect", &Settings::effect);
			}
			ImGui::Separator();
		}

		ImGui::ColorEdit3("Background", (float*)&Settings::backgroundColor);
		ImGui::Checkbox("Transparent Background", &Settings::transparentBackground);
		ImGui::SameLine();
		ImGui::HelpMarker("If you want to capture transparency in recording softwares (like OBS),\nmake sure to also enable something like \"Enable Transparency\" inside the recording software.\nTransparent and edge colors will be darkened unless you also enable FBO and color correction.");
		ImGui::Separator();

		ImGui::Checkbox("Show Canvas", &Settings::showCanvas);
		ImGui::SliderInt("Canvas Line Width", &Settings::canvasLineWidth, 1, 20);
		ImGui::ColorEdit3("Canvas Border Color", (float*)&Settings::canvasBorderColor);
		ImGui::Separator();

		ImGui::SliderInt("Mesh Point Size", &Settings::meshPointSize, 1, 20);
		ImGui::ColorEdit3("Mesh Point Color", (float*)&Settings::meshPointColor);
		ImGui::ColorEdit3("Mesh Point Heighlight Color", (float*)&Settings::meshPointHighlightColor);
		ImGui::ColorEdit3("Mesh Point Selected Color", (float*)&Settings::meshPointSelectedColor);
		ImGui::Separator();

		ImGui::SliderInt("Mesh Point Border Size", &Settings::meshPointBorderSize, 1, 10);
		ImGui::ColorEdit3("Mesh Point Border Color", (float*)&Settings::meshPointBorderColor);
		ImGui::Separator();

		ImGui::SliderInt("Mesh Line Width", &Settings::meshLineWidth, 1, 20);
		ImGui::ColorEdit3("Mesh Line Color", (float*)&Settings::meshLineColor);
		ImGui::Separator();

		ImGui::SliderFloat("Vertex Detection Distance", &Settings::vertexDetectionDistance, 1.0f, 80.0f);
		ImGui::Separator();

		ImGui::DragFloat2("Camera Position", &Camera2D::pos.x);
		ImGui::Separator();

		ImGui::Text("Application average %.3f ms/frame (%d FPS)", 1000.0f / ImGui::GetIO().Framerate, (int)round(ImGui::GetIO().Framerate));

		//ImGui::Text(u8"%s", Localization::getTranslation("test"));
		//ImGui::Text(u8"Japanese: テスト");
	}
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
	if (ImGui::Begin("Meshes") && model)
	{
		//128 max char length, change if you want
		static char nameBuf[128] = "";
		static ModelPart::PartType deformerType;

		if (ImGui::Button("Warp"))
		{
			if (selectedParts.size() && model->checkSameParent(selectedParts))
			{
				ImGui::OpenPopup("New Deformer");
				deformerType = ModelPart::PartType::warpDeformer;
				//might make default name
				nameBuf[0] = '\0';
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Rotation"))
		{
			if (selectedParts.size() && model->checkSameParent(selectedParts))
			{
				ImGui::OpenPopup("New Deformer");
				deformerType = ModelPart::PartType::rotationDeformer;
				nameBuf[0] = '\0';
			}
		}
		ImGui::Separator();

		if (ImGui::BeginPopupModal("New Deformer", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static int boxCount[2] = { 5, 5 };
			switch (deformerType)
			{
			case ModelPart::PartType::warpDeformer:
				ImGui::Text("Warp Deformer Name");

				ImGui::SameLine();
				ImGui::InputText("", nameBuf, 128);
				if (model->checkNameExists(nameBuf))
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DUPLICATE NAME");
				ImGui::Separator();

				ImGui::Text("Box Count (X, Y)");
				ImGui::SameLine();
				ImGui::InputInt2("", boxCount);

				if (ImGui::Button("Add") && nameBuf[0] != '\0')
				{
					if (!model->checkNameExists(nameBuf))
					{
						model->addWarpDeformer(nameBuf, selectedParts, boxCount[0], boxCount[1]);
						nameBuf[0] = '\0';
						ImGui::CloseCurrentPopup();
					}
				}
				break;
			case ModelPart::PartType::rotationDeformer:
				ImGui::Text("Rotation Deformer Name");

				ImGui::SameLine();
				ImGui::InputText("", nameBuf, 128);
				if (model->checkNameExists(nameBuf))
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DUPLICATE NAME");
				ImGui::Separator();

				if (ImGui::Button("Add") && nameBuf[0] != '\0')
				{
					if (!model->checkNameExists(nameBuf))
					{
						model->addRotationDeformer(nameBuf, selectedParts);
						nameBuf[0] = '\0';
						ImGui::CloseCurrentPopup();
					}
				}
				break;
			default:
				break;
			}

			ImGui::SameLine();
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
				nameBuf[0] = '\0';
			}
			ImGui::EndPopup();
		}

		for (int i = static_cast<int>(model->children.size() - 1); i >= 0; i--)
		{
			createModelTree(model->children[i]);
		}
	}
	ImGui::End();

	//details of selected things
	if (ImGui::Begin("Inspector") && model)
	{
		if (selectedParts.size() == 1 && (model->partMap.find(selectedParts[0]) != model->partMap.end()))
		{
			model->partMap[selectedParts[0]]->renderInspector();

			if (model->partMap[selectedParts[0]]->type == ModelPart::PartType::mesh)
			{
				model->showMeshMaskingMenu(selectedParts[0]);
			}
		}
		else
		{
			if (selectedParts.size() > 1)
				ImGui::Text("Selected multiple parts: %d", selectedParts.size());
		}
	}
	ImGui::End();

	//view and edit parameters
	if (ImGui::Begin("Parameters") && model)
	{
		if (ImGui::Button("Reset Values"))
		{
			Log::logInfo("Resetted param values");
			model->resetParams();
		}

		//testing
		ImGui::SameLine();
		if (ImGui::Button("Test Key Value"))
		{
			model->paramMap[model->paramNames[0]]->keyValues.push_back(0.0f);
			model->paramMap[model->paramNames[0]]->keyValues.push_back(10.0f);
			model->paramMap[model->paramNames[0]]->keyValues.push_back(15.0f);
			model->paramMap[model->paramNames[0]]->keyValues.push_back(-20.0f);
			model->paramMap[model->paramNames[0]]->keyValues.push_back(30.0f);
			model->paramMap[model->paramNames[0]]->keyValues.push_back(-30.0f);

			model->paramMap[model->paramNames[6]]->keyValues.push_back(0.0f);
			model->paramMap[model->paramNames[6]]->keyValues.push_back(10.0f);
			model->paramMap[model->paramNames[6]]->keyValues.push_back(-10.0f);
			model->paramMap[model->paramNames[6]]->keyValues.push_back(-5.0f);

		}

		ImGui::Separator();

		for (int i = 0; i < model->paramNames.size(); i++)
		{
			ImGui::SliderParam(model->paramNames[i].c_str(), &model->paramMap[model->paramNames[i]]->value, model->paramMap[model->paramNames[i]]->minValue, model->paramMap[model->paramNames[i]]->maxValue, model->paramMap[model->paramNames[i]]->keyValues);
		}
	}
	ImGui::End();

	Log::draw("Log");
}
