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
	if (eventFramesCount < EVENT_FRAMES)
	{
		Event::anyAction = false;

		window.update();

		//create imgui frame
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		drawImGui();
		//ImGui::ShowDemoWindow();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImVec4 winColor = style->Colors[ImGuiCol_WindowBg];
		style->Colors[ImGuiCol_WindowBg] = { Settings::canvasColor.r, Settings::canvasColor.g, Settings::canvasColor.b, 1.0f };
		//ImGui::PushStyleColor(ImGuiCol_WindowBg, *(ImVec4*)&Settings::backgroundColor);
		if (ImGui::Begin("Model Viewport"))
		{
			ImVec2 mPos = ImGui::GetMousePos();
			ImVec2 offset = ImGui::GetWindowPos();

			Event::viewportMouseCoord = glm::vec2(mPos.x - offset.x, mPos.y - offset.y - ImGui::GetWindowContentRegionMin().y);
			Event::isFocused = ImGui::IsWindowFocused();
			Event::isHovered = ImGui::IsWindowHovered();
			if (model)
			{
				ImVec2 currentFbSize = ImGui::GetContentRegionAvail();
				if (model->fbDimension != *((glm::vec2*)&currentFbSize))
				{
					model->fbDimension = *((glm::vec2*)&currentFbSize);
					model->updateFrameBufferSize((int)currentFbSize.x, (int)currentFbSize.y);
				}

				Camera2D::update();

				int closestVertexIndex = -1;
				int selectedPartNum = -1;
				
				//if spacebar pressed, move camera
				if (Event::keyDown(GLFW_KEY_SPACE) && !draggingVertices)
					panningCamera = true;
				else if (Event::keyReleased(GLFW_KEY_SPACE))
					panningCamera = false;

				if (panningCamera)
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						oldMouseCoord = Event::viewportMouseCoord;
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
					{
						Camera2D::pos += glm::vec2(-Event::viewportMouseCoord.x + oldMouseCoord.x, Event::viewportMouseCoord.y - oldMouseCoord.y) / Camera2D::scale;
						oldMouseCoord = Event::viewportMouseCoord;
					}
				}
				else
				{
					if (Event::isHovered && selectedParts.size())
					{
						closestVertexIndex = model->findClosestVertex(selectedParts, &selectedPartNum);

						if (editingMesh && manualVertexMode == 0)
						{
							//add new vertex
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && closestVertexIndex == -1)
							{
								glm::vec2 lPos = glm::inverse(Camera2D::projection) * glm::vec4(Event::viewportMouseCoord.x * 2.0f / currentFbSize.x - 1.0f, Event::viewportMouseCoord.y * 2.0f / currentFbSize.y - 1.0f, 0.0f, 1.0f);;

								model->meshMap[selectedParts[0]]->addMeshVertex(lPos - model->meshMap[selectedParts[0]]->originalPos, model->atlasWidth, model->atlasHeight);
								Triangulator::triangulate(model->meshMap[selectedParts[0]]->vertices, model->meshMap[selectedParts[0]]->indices);
								model->meshMap[selectedParts[0]]->removeInvisibleTriangles(TextureLoader::tempDirectory, 0);
							}

							if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && closestVertexIndex != -1 && model->meshMap[selectedParts[0]]->vertices.size() > 3)
							{
								model->meshMap[selectedParts[0]]->removeVertex(closestVertexIndex);
								model->meshMap[selectedParts[0]]->removeInvisibleTriangles(TextureLoader::tempDirectory, 0);
							}
						}
						else 
						{
							if (closestVertexIndex != -1)
							{
								if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
								{
									dragMod = Event::mod;

									oldMouseCoord = glm::inverse(Camera2D::projection) * glm::vec4(Event::viewportMouseCoord.x * 2.0f / currentFbSize.x - 1.0f, Event::viewportMouseCoord.y * 2.0f / currentFbSize.y - 1.0f, 0.0f, 1.0f);

									//if clicked vertex is not in selected vertices, clear
									if (GLFW_MOD_CONTROL != dragMod && std::find(model->selectedVertices.begin(), model->selectedVertices.end(), VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex)) == model->selectedVertices.end())
									{
										model->selectedVertices.clear();
									}

									//if selected vertex's part has parameters or is a rotation deformer, and not in mesh edit mode
									if ((model->partMap[selectedParts[selectedPartNum]]->keyforms.size() || model->partMap[selectedParts[selectedPartNum]]->type == ModelPart::PartType::rotationDeformer) && !editingMesh)
									{
										model->selectedVertices.emplace_back(VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex));
										model->updateOriginalVertexPositions();
									}
									//if manual move vertex mode
									else if (editingMesh && manualVertexMode == 1)
									{
										model->selectedVertices.emplace_back(VertexSpecifier(selectedParts[selectedPartNum], closestVertexIndex));
										model->updateOriginalMeshPositions();
									}
								}
							}
							//if clicked on nothing
							else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								model->selectedVertices.clear();
							}
						}
					}

					if (Event::isFocused && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && GLFW_MOD_CONTROL != dragMod)
					{
						if (editingMesh && manualVertexMode == 1)
						{
							model->moveMeshVertices(oldMouseCoord, dragMod);
							draggingVertices = true;
						}
						else
						{
							if (model->selectedVertices.size())
							{
								//check if keyvalues are on keyforms of all parts
								bool onKeyform = true;

								//for each selected part
								for (int i = 0; i < selectedParts.size() && onKeyform; i++)
								{
									//if rotation deformer with no keyforms, skip
									if (model->partMap[selectedParts[i]]->type == ModelPart::PartType::rotationDeformer && model->partMap[selectedParts[i]]->keyforms.size() == 0)
										continue;

									//for each param name in part
									for (int j = 0; j < model->partMap[selectedParts[i]]->paramNames.size(); j++)
									{
										//check if paramValue is in the keyvalues
										if (std::find(model->partMap[selectedParts[i]]->paramKeyvalues[j].begin(), model->partMap[selectedParts[i]]->paramKeyvalues[j].end(), model->paramValues[model->partMap[selectedParts[i]]->paramNames[j]]) == model->partMap[selectedParts[i]]->paramKeyvalues[j].end())
										{
											onKeyform = false;
											break;
										}
									}
								}

								if (onKeyform)
								{
									model->moveSelectedVertices(oldMouseCoord, dragMod);
									draggingVertices = true;
								}
							}
						}
					}
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					draggingVertices = false;
					if (!Event::keyDown(GLFW_KEY_SPACE))
						panningCamera = false;
				}

				//check if mouse should detect which mesh it is on
				if (!draggingVertices && !panningCamera && Event::isHovered)
					model->detectMouseHover = true;
				else
					model->detectMouseHover = false;

				if (editingMesh)
				{
					model->meshMap[selectedParts[0]]->updateVertexData();
					model->renderEditMesh(selectedParts[0]);

					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					model->renderMeshVertices(selectedParts[0]);

					if (forceShowVertices)
						model->forceRenderVertices(selectedParts[0]);
					if (manualVertexMode == 1)
						model->renderSelectedVertices();
					if (closestVertexIndex != -1)
						model->renderClosestVertex(selectedParts[selectedPartNum], closestVertexIndex);
				}
				else
				{
					model->update();
					model->render();

					//render lines and points
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

					//if hovering over mesh that is not selected, and no closest vertex
					if (Event::isHovered && !panningCamera)
					{
						if (model->mouseHoveredID > -1 && std::find(selectedParts.begin(), selectedParts.end(), model->modelMeshes[model->mouseHoveredID]->name) == selectedParts.end() && closestVertexIndex == -1)
						{
							model->renderHighlightedMesh();
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								model->selectedVertices.clear();
								selectedParts.clear();

								selectedParts.push_back(model->modelMeshes[model->mouseHoveredID]->name);
							}
						}
						else if (model->mouseHoveredID == -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && closestVertexIndex == -1)
						{
							model->selectedVertices.clear();
							selectedParts.clear();
						}


					}


					if (selectedParts.size())
					{
						for (int i = 0; i < selectedParts.size(); i++)
						{
							model->renderMeshVertices(selectedParts[i]);
						}

						model->renderSelectedVertices();

						if (closestVertexIndex != -1)
							model->renderClosestVertex(selectedParts[selectedPartNum], closestVertexIndex);
					}
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				ImGui::Image((void*)(size_t)model->modelTexTrueColorBuffer, currentFbSize);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
		style->Colors[ImGuiCol_WindowBg] = winColor;
		//ImGui::PopStyleColor();

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
	editingMesh = false;
	draggingVertices = false;
	panningCamera = false;

	double startTime = glfwGetTime();
	model = std::make_shared<Model>();
	if (TextureLoader::loadPsdFile(fileName, model))
	{
		//TextureLoader::loadTexture(&model->textureID, "saves/testExports/textureAtlas.png", &model->atlasWidth, &model->atlasHeight, &model->atlasNrChannels);
		Log::info("Took %f seconds to load PSD", glfwGetTime() - startTime);

		model->generateDefaltParams();
		model->updatePartMap();
		model->bindUniformTextures();

		Camera2D::pos = glm::vec2();
		Camera2D::scale = 1.0f;
	}
	else
		Log::error("Failed to load psd");
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
	//temporary files for mesh texture
	TextureLoader::tempDirectory = std::filesystem::temp_directory_path();

	window.init();
	Event::GLFWWin = window.getWindow();

	glfwSetCursorPosCallback(window.getWindow(), Event::cursor_position_callback);
	glfwSetMouseButtonCallback(window.getWindow(), Event::mouse_button_callback);
	glfwSetFramebufferSizeCallback(window.getWindow(), Event::framebuffer_size_callback);
	glfwSetScrollCallback(window.getWindow(), Event::scroll_callback);
	glfwSetKeyCallback(window.getWindow(), Event::key_callback);

	//stbi_set_flip_vertically_on_load(true);
	//stbi_flip_vertically_on_write(true);

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
	style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.9f, 0.9f, 0.9f, 0.2f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);

	/*
	int temp = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
	std::cout << "GL_MAX_TEXTURE_SIZE: " << temp << std::endl;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &temp);
	std::cout << "GL_MAX_ARRAY_TEXTURE_LAYERS: " << temp << std::endl;
	*/
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

			if (ImGui::IsItemClicked() && !editingMesh)
			{
				if (GLFW_MOD_CONTROL != Event::mod)
				{
					selectedParts.clear();
					model->selectedVertices.clear();
					alreadySelected = false;
				}
				if (alreadySelected)
					selectedParts.erase(remove(selectedParts.begin(), selectedParts.end(), currentPart->name), selectedParts.end());
				else
					selectedParts.push_back(currentPart->name);

				//might implement some way to keep selected vertices if mesh is still selected
				model->selectedVertices.clear();
			}
			break;
		//for now, treat all of them the same
		case ModelPartUI::PartType::divider:
		case ModelPartUI::PartType::warpDeformer:
		case ModelPartUI::PartType::rotationDeformer:
			nodeOpen = ImGui::TreeNodeEx(currentPart->name.c_str(), nodeFlags);
			if (ImGui::IsItemClicked() && !editingMesh)
			{
				if (GLFW_MOD_CONTROL != Event::mod && !ImGui::IsItemToggledOpen())
				{
					selectedParts.clear();
					model->selectedVertices.clear();
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

	if (ImGui::IsItemClicked() && !editingMesh)
	{
		if (GLFW_MOD_CONTROL != Event::mod)
		{
			selectedParts.clear();
			model->selectedVertices.clear();
			alreadySelected = false;
		}
		if (alreadySelected)
			selectedParts.erase(remove(selectedParts.begin(), selectedParts.end(), currentPart->name), selectedParts.end());
		else
			selectedParts.push_back(currentPart->name);

		//might implement some way to keep selected vertices if mesh is still selected
		model->selectedVertices.clear();
	}

	//drag and drop stuff
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
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
			model->partMap[partName]->basePos = temp;

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
			{
				initializeModelFromPsd("tempPsdTest/testModel.psd");

				//simple test
				model->update();

				selectedParts.push_back(model->modelMeshes[model->modelMeshes.size() - 1]->name);
				model->addWarpDeformer("TestWarp", selectedParts, 5, 5);
				selectedParts.clear();
				selectedParts.push_back(model->modelMeshes[model->modelMeshes.size() - 2]->name);
				model->addRotationDeformer("TestRotation", selectedParts);
				selectedParts.clear();

				model->partMap["headMain"]->addKeyform("headX", 0.0f);
				model->partMap["headMain"]->addKeyform("headX", model->paramMap["headX"]->maxValue);
				model->partMap["headMain"]->addKeyform("headY", 0.0f);
				model->partMap["headMain"]->addKeyform("headY", model->paramMap["headY"]->maxValue);
				model->partMap["headMain"]->keyforms[0].deltaPosition = glm::vec2(0.0f, 0.0f);
				model->partMap["headMain"]->keyforms[1].deltaPosition = glm::vec2(100.0f, 0.0f);
				model->partMap["headMain"]->keyforms[2].deltaPosition = glm::vec2(0.0f, 100.0f);
				model->partMap["headMain"]->keyforms[3].deltaPosition = glm::vec2(100.0f, 100.0f);

				//testing keyform values
				/*
				model->update();
				model->addKeyform("headMain", "headX", model->paramMap["headX"]->minValue + 5.0f);
				model->addKeyform("headMain", "headX", model->paramMap["headX"]->maxValue - 5.0f);
				model->addKeyform("headMain", "headY", model->paramMap["headY"]->minValue + 5.0f);
				model->addKeyform("headMain", "headY", model->paramMap["headY"]->maxValue - 5.0f);
				model->addKeyform("headMain", "headZ", model->paramMap["headZ"]->minValue + 5.0f);
				model->addKeyform("headMain", "headZ", model->paramMap["headZ"]->maxValue - 5.0f);
				model->partMap["headMain"]->keyforms[0].position = glm::vec2(-100.0f, 0.0f);
				model->partMap["headMain"]->keyforms[1].position = glm::vec2(100.0f, 0.0f);
				model->partMap["headMain"]->keyforms[2].position = glm::vec2(-100.0f, 100.0f);
				model->partMap["headMain"]->keyforms[3].position = glm::vec2(100.0f, 100.0f);
				model->partMap["headMain"]->keyforms[4].position = glm::vec2(-100.0f, 0.0f);
				model->partMap["headMain"]->keyforms[5].position = glm::vec2(100.0f, 0.0f);
				model->partMap["headMain"]->keyforms[6].position = glm::vec2(-100.0f, 100.0f);
				model->partMap["headMain"]->keyforms[7].position = glm::vec2(100.0f, 100.0f);
				model->partMap["headMain"]->keyforms[4].scale = glm::vec2(2.0f, 1.0f);
				model->partMap["headMain"]->keyforms[5].scale = glm::vec2(2.0f, 1.0f);
				model->partMap["headMain"]->keyforms[6].scale = glm::vec2(1.0f, 2.0f);
				model->partMap["headMain"]->keyforms[7].scale = glm::vec2(1.0f, 2.0f);
				*/

				model->calculateModelParam();
			}
			if (ImGui::MenuItem("Open test dog"))
				initializeModelFromPsd("tempPsdTest/testDog.psd");
			if (ImGui::MenuItem("Open test model krita"))
				initializeModelFromPsd("tempPsdTest/testModelKrita.psd");
			if (ImGui::MenuItem("Open fluffy"))
				initializeModelFromPsd("tempPsdTest/fluffy.psd");
			if (ImGui::MenuItem("Open mini test 2"))
				initializeModelFromPsd("tempPsdTest/miniTest2.psd");
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
			Log::warning("Implementing reading ftd files later");
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

		if (ImGui::Button("Temp Mesh Test") && meshGeneratorValid && model)
		{
			for (int i = 0; i < model->modelMeshes.size(); i++)
				model->modelMeshes[i]->autoMesh(TextureLoader::tempDirectory, model->atlasWidth, model->atlasHeight, 6, 8, 30, 40, 0);
		}

		static int boxCount[2] = { 5, 5 };
		if (ImGui::Button("Test Box Vertices") && meshGeneratorValid && model)
		{
			ImGui::OpenPopup("Generate Box Vertices");
		}


		if (ImGui::BeginPopupModal("Generate Box Vertices", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputInt2("Box Count (X, Y)", boxCount);

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

		if (model && ImGui::Button("Screenshot"))
			model->screenshot = true;
	}
	ImGui::End();

	//log
	Log::draw("Log");

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

		ImGui::ColorEdit3("Canvas Color", (float*)&Settings::canvasColor);
		ImGui::ColorEdit3("Background", (float*)&Settings::backgroundColor);
		ImGui::Separator();

		ImGui::Checkbox("Show Frame", &Settings::showFrame);
		ImGui::DragInt("Frame Line Width", &Settings::frameLineWidth);
		ImGui::ColorEdit3("Frame Border Color", (float*)&Settings::frameBorderColor);
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

		ImGui::ColorEdit3("Warp Deformer Color", (float*)&Settings::warpDeformerColor);
		ImGui::ColorEdit3("Rotation Deformer Color", (float*)&Settings::rotationDeformerColor);
		ImGui::DragInt("Rotation Deformer Width", &Settings::rotationDeformerWidth);
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
			if (openDeformerMenu)
				ImGui::SetKeyboardFocusHere();

			static int boxCount[2] = { 5, 5 };
			switch (deformerType)
			{
			case ModelPart::PartType::warpDeformer:
				ImGui::InputText("Warp Deformer Name", nameBuf, 128);
				if (model->checkNameExists(nameBuf))
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DUPLICATE NAME");
				ImGui::Separator();

				ImGui::InputInt2("Box Count (X, Y)", boxCount);

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
				ImGui::InputText("Rotation Deformer Name", nameBuf, 128);
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
			if (editingMesh)
			{
				if (ImGui::RadioButton("Add/remove vertices", &manualVertexMode, 0))
					model->selectedVertices.clear();
				ImGui::SameLine();
				if (ImGui::RadioButton("Move vertices", &manualVertexMode, 1))
					model->selectedVertices.clear();

				ImGui::Checkbox("Force show vertices", &forceShowVertices);

				if (ImGui::Button("Done"))
				{
					model->selectedVertices.clear();
					editingMesh = false;
				}
			}
			else
			{
				model->partMap[selectedParts[0]]->renderInspector();

				if (model->partMap[selectedParts[0]]->type == ModelPart::PartType::mesh)
				{
					//maybe move this somewhere else
					if (ImGui::Button("Testing mesh generation"))
						ImGui::OpenPopup("Auto Mesh Generator");

					if (ImGui::BeginPopupModal("Auto Mesh Generator", NULL, ImGuiWindowFlags_AlwaysAutoResize))
					{
						static int edgeOut = 6, edgeIn = 8, edgeSpacing = 30, insideSpacing = 50, alphaThreshold = 0;
						ImGui::InputInt("Outer Edge", &edgeOut);
						ImGui::InputInt("Inner Edge", &edgeIn);
						ImGui::InputInt("Edge Spacing", &edgeSpacing);
						ImGui::InputInt("Inside Spacing", &insideSpacing);
						ImGui::InputInt("Alpha Threshold", &alphaThreshold);

						if (edgeOut < 2) edgeOut = 2;
						if (edgeIn < 1) edgeIn = 1;
						if (edgeSpacing < 1) edgeSpacing = 1;
						if (insideSpacing < 1) insideSpacing = 1;
						alphaThreshold = std::clamp(alphaThreshold, 0, 254);

						if (ImGui::Button("Auto Mesh Test"))
							model->meshMap[selectedParts[0]]->autoMesh(TextureLoader::tempDirectory, model->atlasWidth, model->atlasHeight, edgeOut, edgeIn, edgeSpacing, insideSpacing, alphaThreshold);
						if (ImGui::Button("Close"))
							ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}

					ImGui::SameLine();
					if (ImGui::Button("Edit Mesh"))
					{
						editingMesh = true;
						model->selectedVertices.clear();

						auto m = std::dynamic_pointer_cast<ModelMesh>(model->partMap[selectedParts[0]]);
						m->startMeshEdit();
					}

					model->showMeshClippingMenu(selectedParts[0]);
				}
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
			if (ImGui::Button("2##Add 2 Keyvalues"))
			{
				//check if param name exists
				if (model->paramMap.find(selectedParam) != model->paramMap.end())
				{
					for (int i = 0; i < selectedParts.size(); i++)
					{
						model->partMap[selectedParts[i]]->addKeyform(selectedParam, model->paramMap[selectedParam]->minValue);
						model->partMap[selectedParts[i]]->addKeyform(selectedParam, model->paramMap[selectedParam]->maxValue);
					}
					model->calculateModelParam();
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Add 2 Keyvalues");

			if (ImGui::Button("3##Add 3 Keyvalues"))
			{
				if (model->paramMap.find(selectedParam) != model->paramMap.end())
				{
					for (int i = 0; i < selectedParts.size(); i++)
					{
						model->partMap[selectedParts[i]]->addKeyform(selectedParam, model->paramMap[selectedParam]->minValue);
						model->partMap[selectedParts[i]]->addKeyform(selectedParam, model->paramMap[selectedParam]->defaultValue);
						model->partMap[selectedParts[i]]->addKeyform(selectedParam, model->paramMap[selectedParam]->maxValue);
					}
					model->calculateModelParam();
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Add 3 Keyvalues");

			if (ImGui::Button("M##Manual Keyvalues") && model->paramMap.find(selectedParam) != model->paramMap.end())
				manualKeyEditing = true;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Manual Keyvalues");

			if (manualKeyEditing)
				ImGui::OpenPopup("Manual Keyvalues");

			if (ImGui::BeginPopupModal("Manual Keyvalues", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputFloat("Keyvalue", &manualKeyvalue);
				ImGui::SameLine();
				if (ImGui::Button("Add"))
				{
					for (int i = 0; i < selectedParts.size(); i++)
						model->partMap[selectedParts[i]]->addKeyform(selectedParam, std::clamp(manualKeyvalue, model->paramMap[selectedParam]->minValue, model->paramMap[selectedParam]->maxValue));
					model->calculateModelParam();
				}

				if (selectedParts.size() == 1)
				{
					ImGui::Text("Remove keyvalues");
					auto temp = std::find(model->partMap[selectedParts[0]]->paramNames.begin(), model->partMap[selectedParts[0]]->paramNames.end(), selectedParam);
					if (temp != model->partMap[selectedParts[0]]->paramNames.end())
					{
						int paramIndex = static_cast<int>(temp - model->partMap[selectedParts[0]]->paramNames.begin());
						for (int i = 0; i < model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex].size(); i++)
						{
							if (ImGui::Button(std::to_string(model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex][i]).c_str()))
							{
								model->partMap[selectedParts[0]]->removeKeyform(selectedParam, i);
								model->calculateModelParam();
							}
							if (i + 1 < model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex].size())
								ImGui::SameLine();
						}
					}
				}

				if (ImGui::Button("Close"))
				{
					manualKeyEditing = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (ImGui::Button("D##Delete Keyvalues") && model->paramMap.find(selectedParam) != model->paramMap.end())
			{
				if (selectedParts.size())
				{
					std::vector <std::string> commonParamNames;
					if (selectedParts.size() == 1)
						commonParamNames = model->partMap[selectedParts[0]]->paramNames;
					else
					{
						for (int i = 0; i < model->partMap[selectedParts[0]]->paramNames.size(); i++)
						{
							bool inAll = true;
							for (int j = 1; j < selectedParts.size(); j++)
							{
								if (std::find(model->partMap[selectedParts[j]]->paramNames.begin(), model->partMap[selectedParts[j]]->paramNames.end(), model->partMap[selectedParts[0]]->paramNames[i]) == model->partMap[selectedParts[j]]->paramNames.end())
								{
									inAll = false;
									break;
								}
							}
							if (inAll)
								commonParamNames.push_back(model->partMap[selectedParts[0]]->paramNames[i]);
						}
					}

					for (int i = 0; i < commonParamNames.size(); i++)
					{
						for (int j = 0; j < selectedParts.size(); j++)
						{
							model->partMap[selectedParts[j]]->removeParameter(commonParamNames[i]);
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Delete Keyvalues");

			if (ImGui::Button("R##Reset Keyvalues"))
			{
				model->resetParams();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Reset Keyvalues");

			ImGui::EndMenuBar();
		}

		for (int i = 0; i < model->paramNames.size(); i++)
		{
			//common keyvalues between all selected parts
			std::vector<float> commonKeyvalues;
			if (selectedParts.size())
			{
				auto it = std::find(model->partMap[selectedParts[0]]->paramNames.begin(), model->partMap[selectedParts[0]]->paramNames.end(), model->paramNames[i]);
				
				//if first selected object does have param name
				if (it != model->partMap[selectedParts[0]]->paramNames.end())
				{
					int paramIndex = static_cast<int>(it - model->partMap[selectedParts[0]]->paramNames.begin());

					//if only one part selected, copy vector
					if (selectedParts.size() == 1)
					{
						commonKeyvalues = model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex];
					}
					//if multiple, seach for common keyvalues
					else
					{
						//for each keyvalue in first selected part
						for (int j = 0; j < model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex].size(); j++)
						{
							bool allParts = true;

							//for each selected part
							for (int k = 1; k < selectedParts.size(); k++)
							{
								auto it2 = std::find(model->partMap[selectedParts[k]]->paramNames.begin(), model->partMap[selectedParts[k]]->paramNames.end(), model->paramNames[i]);

								//if selected part doesn't also have same param name
								if (it2 == model->partMap[selectedParts[k]]->paramNames.end())
								{
									allParts = false;
									break;
								}

								int paramIndex2 = static_cast<int>(it2 - model->partMap[selectedParts[k]]->paramNames.begin());

								if (std::find(model->partMap[selectedParts[k]]->paramKeyvalues[paramIndex2].begin(), model->partMap[selectedParts[k]]->paramKeyvalues[paramIndex2].end(), model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex][j]) == model->partMap[selectedParts[k]]->paramKeyvalues[paramIndex2].end())
								{
									allParts = false;
									break;
								}
							}

							if (allParts)
								commonKeyvalues.push_back(model->partMap[selectedParts[0]]->paramKeyvalues[paramIndex][j]);
						}
					}
				}
			}

			if (ImGui::SliderParam(model->paramNames[i].c_str(), &model->paramValues[model->paramNames[i]], model->paramMap[model->paramNames[i]]->minValue, model->paramMap[model->paramNames[i]]->maxValue, model->paramMap[model->paramNames[i]]->keyValues, commonKeyvalues, selectedParam == model->paramNames[i]))
				selectedParam = model->paramNames[i];
		}
	}
	ImGui::End();
}
