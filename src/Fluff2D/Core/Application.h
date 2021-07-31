#pragma once

#include <memory>

#include "imgui_filebrowser/imfilebrowser.h"
#include "Window.h"
#include "../Events/Event.h"
#include "../Renderer/Model.h"
#include "../Renderer/ModelMesh.h"
#include "SaveSystem.h"
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
	unsigned int canvasVAO = 0, canvasVBO = 0, canvasEBO = 0;
	glm::vec2 canvasCoords[4] = { glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f) };
	unsigned int canvasIndices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };

	ImGui::FileBrowser fileBrowser;

	void checkRunning();

	void setCanvasCoords();
	void renderCanvas();
	
	//for docking
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	//for model parts panel
	ImGuiTreeNodeFlags modelPartsFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	std::vector<std::string> selectedParts;

	void createModelTree(std::shared_ptr<ModelPartUI> currentPart);
};

