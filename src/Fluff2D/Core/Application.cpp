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

	if (Event::anyAction)
		eventFramesCount = 0;
	if (eventFramesCount < EVENT_FRAMES || Settings::effect)
	{
		Event::anyAction = false;

		Camera2D::update();

		//don't need to clear window when transparent back since model FBO overwrites everything
		if (!model || !Settings::transparentBackground)
			window.update();

		if (model)
		{
			model->update();

			int closestVertexIndex = -1;
			int selectedPartNum = -1;
			if (!Event::io->WantCaptureMouse)
			{
				if (selectedParts.size())
				{
					//testing stuff, move somewhere else later
					closestVertexIndex = model->findClosestVertex(selectedParts, &selectedPartNum);
					if (closestVertexIndex != -1)
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						{
							oldMouseCoord = glm::inverse(Camera2D::projection) * glm::vec4(ImGui::GetMousePos().x * 2.0f / Window::width - 1.0f, ImGui::GetMousePos().y * -2.0f / Window::height + 1.0f, 0.0f, 1.0f);;
							//if ctrl key pressed, add to selected vertices
							if (GLFW_MOD_CONTROL == Event::mod)
							{
								model->selectedVertices.emplace_back(VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex));
								model->initialVerticesPos[&model->selectedVertices.emplace_back(VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex))] = model->partMap[selectedParts[selectedPartNum]]->vertices[closestVertexIndex].position;
							}
							else
							{
								//if clicked vertex is not in selected vertices, clear
								if (std::find(model->selectedVertices.begin(), model->selectedVertices.end(), VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex)) == model->selectedVertices.end())
								{
									model->selectedVertices.clear();
									model->initialVerticesPos.clear();
									model->selectedVertices.emplace_back(VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex));
									model->initialVerticesPos[&model->selectedVertices.emplace_back(VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex))] = model->partMap[selectedParts[selectedPartNum]]->vertices[closestVertexIndex].position;
								}
							}
							model->updateOriginalVertexPositions();
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
					//model->updateOriginalVertexPositions();
					draggingVertices = false;
				}

				//check if mouse should detect which mesh it is on
				if (!draggingVertices && mouseInWindow())
					model->detectMouseHover = true;
			}

			model->render();

			//render lines and points
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			//if hovering over mesh that is not selected, and no closest vertex
			if (model->mouseHoveredID > -1 && std::find(selectedParts.begin(), selectedParts.end(), model->modelMeshes[model->mouseHoveredID]->name) == selectedParts.end() && closestVertexIndex == -1)
			{
				model->renderHighlightedMesh();
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					model->selectedVertices.clear();
					model->initialVerticesPos.clear();
					selectedParts.clear();

					selectedParts.push_back(model->modelMeshes[model->mouseHoveredID]->name);
				}

			}

			if (selectedParts.size())
			{
				model->shader.setInt("mode", 1);

				for (int i = 0; i < selectedParts.size(); i++)
				{
					model->renderMeshVertice(selectedParts[i]);
				}

				model->renderSelectedVertices();

				if (closestVertexIndex != -1)
					model->renderClosestVertex(selectedParts[selectedPartNum], closestVertexIndex);

			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (Event::keyPressed(GLFW_KEY_ESCAPE))
			drawMenu ^= true;

		//create imgui frame
		if (drawMenu)
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			drawImGui();

			//testing
			//ImGui::ShowDemoWindow();

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
		eventFramesCount++;
	}
	else
		std::this_thread::sleep_for(std::chrono::milliseconds((int)((1.0f / 60.0f - Event::deltaTime) * 1000.0f)));

	checkRunning();
}

void Application::initializeModelFromPsd(const char* fileName)
{
	Event::anyAction = true;
	selectedParts.clear();

	double startTime = glfwGetTime();
	model = std::make_shared<Model>();
	if (TextureLoader::loadPsdFile(fileName, model))
	{
		TextureLoader::loadTexture(&model->textureID, "saves/testExports/textureAtlas.png", &model->atlasWidth, &model->atlasHeight, &model->atlasNrChannels);
		Log::logInfo("Took %f seconds to load PSD", glfwGetTime() - startTime);

		model->generateDefaltParams();
		model->updatePartMap();
		model->bindUniformTextures();
	}
	else
		Log::logError("Failed to load psd");
}

void Application::saveModel()
{
	Event::anyAction = true;
	if (model)
		SaveSystem::saveModel(model, version);
}

int Application::loadModel(const char* filePath)
{
	Event::anyAction = true;
	//idk what to do here, this is junk for now don't call
	std::cout << "Don't call loadModel, yet" << std::endl;

	return 0;
}

void Application::init()
{
	window.init();
	Event::GLFWWin = window.getWindow();

	glfwSetCursorPosCallback(window.getWindow(), Event::cursor_position_callback);
	glfwSetMouseButtonCallback(window.getWindow(), Event::mouse_button_callback);
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
	glPointSize(static_cast<GLfloat>(Settings::meshPointSize));

	//popup bg color
	//also changed fade speed to instant
	//in imgui.cpp in ImGui::NewFrame(), search for "Background darkening/whitening"
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.9f, 0.9f, 0.9f, 0.2f);
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
			}
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
					model->selectedVertices.clear();
					model->initialVerticesPos.clear();
					alreadySelected = false;
				}
				if (!ImGui::IsItemToggledOpen())
				{
					if (alreadySelected)
						selectedParts.erase(remove(selectedParts.begin(), selectedParts.end(), currentPart->name), selectedParts.end());
					else
						selectedParts.push_back(currentPart->name);
				}

				model->selectedVertices.clear();
				model->initialVerticesPos.clear();
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

void Application::createModelTree(std::shared_ptr<ModelPart> currentPart)
{
	bool alreadySelected = std::find(selectedParts.begin(), selectedParts.end(), currentPart->name) != selectedParts.end();
	bool nodeOpen = false;
	ImGuiTreeNodeFlags nodeFlags = modelPartsFlags;
	if (alreadySelected)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	//may or may not replace with icons
	const char* header;
	switch (currentPart->type)
	{
	case ModelPart::PartType::mesh:
		header = "[M] ";
		nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		break;
	case ModelPart::PartType::warpDeformer:
		header = "[W] ";
		break;
	case ModelPart::PartType::rotationDeformer:
		header = "[R] ";
		break;
	default:
		header = "[UNKNOWN]";
		break;
	}


	nodeOpen = ImGui::TreeNodeEx((header + currentPart->name).c_str(), nodeFlags);

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

	//drag and drop stuff
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		//std::cout << "Source: " << currentPart->name << std::endl;

		ImGui::SetDragDropPayload("ModelPart", currentPart->name.c_str(), currentPart->name.length() + 1);
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ModelPart"))
		{
			//the dropped's name
			char* partName = (char*)payload->Data;

			//translates position, rotation not implemented
			glm::vec2 temp = glm::inverse(currentPart->parent->transform) * (model->partMap[partName]->parent->transform * glm::vec4(model->partMap[partName]->pos, 0.0f, 1.0f));
			model->partMap[partName]->pos = temp;

			//remove dropped from parent and assign to new parent
			model->partMap[partName]->parent->children.erase(
				remove(model->partMap[partName]->parent->children.begin(), model->partMap[partName]->parent->children.end(), model->partMap[partName]),
				model->partMap[partName]->parent->children.end());
			currentPart->parent->children.insert((std::find(currentPart->parent->children.begin(), currentPart->parent->children.end(), currentPart) + 1), model->partMap[partName]);
			model->partMap[partName]->parent = currentPart->parent;
		}
		ImGui::EndDragDropTarget();
	}

	if (currentPart->type != ModelPart::PartType::mesh && nodeOpen)
	{
		for (int i = static_cast<int>(currentPart->children.size() - 1); i >= 0; i--)
		{
			createModelTree(currentPart->children[i]);
		}
		ImGui::TreePop();
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
			if (ImGui::MenuItem("Open test model csp"))
				initializeModelFromPsd("tempPsdTest/testModel.psd");
			if (ImGui::MenuItem("Open test model krita"))
				initializeModelFromPsd("tempPsdTest/testModelKrita.psd");
			if (ImGui::MenuItem("Open fluffy"))
				initializeModelFromPsd("tempPsdTest/fluffy.psd");
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
			if (model->partMap.find(selectedParts[i]) == model->partMap.end())
				continue;
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
					if (model->partMap.find(selectedParts[i]) == model->partMap.end())
						continue;
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

		if (model && ImGui::Button("Test Button"))
			model->screenshot = true;
	}
	ImGui::End();

	//log
	ImGui::Begin("Log");
	ImGui::End();

	//general
	if (ImGui::Begin("General"))
	{
		//bunch of settings and stuff with frame rate so might as well
		Event::anyAction = true;

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

		ImGui::DragInt("Font Size", &Settings::fontSize);
		if (ImGui::Button("Update Font"))
			queueFontChange = true;
		ImGui::Separator();

		if (model)
		{
			ImGui::Checkbox("Color Correction", &Settings::colorCorrection);
			ImGui::SameLine();
			ImGui::HelpMarker("It looks weird in this window with transparent background,\nbut fixes itself in the recording software.");
			ImGui::Checkbox("Effect", &Settings::effect);
			ImGui::Separator();
		}

		ImGui::ColorEdit3("Background", (float*)&Settings::backgroundColor);
		ImGui::Checkbox("Transparent Background", &Settings::transparentBackground);
		ImGui::SameLine();
		ImGui::HelpMarker("If you want to capture transparency in recording softwares (like OBS),\nmake sure to also enable something like \"Enable Transparency\" inside the recording software.\nTransparent colors will be darkened unless you also enable color correction.");
		ImGui::Separator();

		ImGui::Checkbox("Show Canvas", &Settings::showCanvas);
		ImGui::DragInt("Canvas Line Width", &Settings::canvasLineWidth);
		ImGui::ColorEdit3("Canvas Border Color", (float*)&Settings::canvasBorderColor);
		ImGui::Separator();

		ImGui::DragInt("Mesh Point Size", &Settings::meshPointSize);
		ImGui::ColorEdit3("Mesh Point Color", (float*)&Settings::meshPointColor);
		ImGui::ColorEdit3("Mesh Point Heighlight Color", (float*)&Settings::meshPointHighlightColor);
		ImGui::ColorEdit3("Mesh Point Selected Color", (float*)&Settings::meshPointSelectedColor);
		ImGui::Separator();

		ImGui::DragInt("Mesh Point Border Size", &Settings::meshPointBorderSize);
		ImGui::ColorEdit3("Mesh Point Border Color", (float*)&Settings::meshPointBorderColor);
		ImGui::Separator();

		ImGui::DragInt("Mesh Line Width", &Settings::meshLineWidth);
		ImGui::ColorEdit3("Mesh Line Color", (float*)&Settings::meshLineColor);
		ImGui::ColorEdit3("Mesh Line Highlight Color", (float*)&Settings::meshHighlightColor);
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
	ImGui::Begin("Model Parts", NULL, ImGuiWindowFlags_MenuBar);
	if (model)
	{
		for (int i = static_cast<int>(model->layerStructure.size() - 1); i >= 0; i--)
		{
			createModelTree(model->layerStructure[i]);
		}
	}
	ImGui::End();

	//the deformer and mesh structure of the model
	if (ImGui::Begin("Meshes", NULL, ImGuiWindowFlags_MenuBar) && model)
	{
		//128 max char length, change if you want
		static char nameBuf[128] = "";
		static ModelPart::PartType deformerType;
		bool openDeformerMenu = false;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::Button("Warp"))
			{
				openDeformerMenu = true;
				deformerType = ModelPart::PartType::warpDeformer;
			}
			if (ImGui::Button("Rotation"))
			{
				openDeformerMenu = true;
				deformerType = ModelPart::PartType::rotationDeformer;
			}
			ImGui::EndMenuBar();
		}

		if (openDeformerMenu && selectedParts.size() && model->checkSameParent(selectedParts))
		{
			nameBuf[0] = '\0';
			ImGui::OpenPopup("New Deformer");
		}

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
				model->showMeshClippingMenu(selectedParts[0]);
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
	if (ImGui::Begin("Parameters", NULL, ImGuiWindowFlags_MenuBar) && model)
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Add Keyvalues"))
			{
				ImGui::MenuItem("Add 2 Keyvalues");
				ImGui::MenuItem("Add 3 Keyvalues");
				ImGui::MenuItem("Add Manual Keyvalues");
				ImGui::MenuItem("Delete Keyvalues");
				ImGui::EndMenu();
			}
			ImGui::Button("Test");
			ImGui::EndMenuBar();
		}

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

bool Application::mouseInWindow()
{
	int w, h;
	glfwGetWindowSize(window.getWindow(), &w, &h);
	ImVec2 mPos = ImGui::GetMousePos();

	if (mPos.x >= 0.0f && (int)mPos.x <= w && mPos.y >= 0.0f && (int)mPos.y <= h)
		return true;
	return false;
}
