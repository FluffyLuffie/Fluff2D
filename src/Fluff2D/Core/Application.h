#pragma once

#include <memory>

#include "imgui_filebrowser/imfilebrowser.h"

#include "SaveSystem.h"
#include "Window.h"
#include "../Renderer/Model.h"
#include "../Renderer/ModelMesh.h"
#include "../Renderer/TextureLoader.h"

class Application
{
public:
	Application();
	~Application();

	//a is alpha, b is beta
	//then the version number
	//for example, a-1-0-4 is Alpha 1.0.4
	int version = ('a' << 24) | ((0 & 0xFF) << 16) | ((0 & 0xFF) << 8) | (0 & 0xFF);

	Window window;
	std::shared_ptr<Model> model;

	void init();
	void update();

	void initializeModelFromPsd(const char* fileName);

	void saveModel();
	int loadModel(const char* filePath);

	bool isRunning = true;
private:
	ImVec2 oldMouseCoord = ImVec2();

	ImGui::FileBrowser fileBrowser;

	bool queueFontChange = false;

	void checkRunning();
	
	//for docking
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	//for model parts panel
	ImGuiTreeNodeFlags modelPartsFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	std::vector<std::string> selectedParts;

	void createModelTree(std::shared_ptr<ModelPartUI> currentPart);
	void createModelTree(std::shared_ptr<ModelPart> currentPart);

	void drawImGui();
};

