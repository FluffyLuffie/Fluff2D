#pragma once

#include <memory>
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

	Window window;
	Model model;

	void init();
	void update();

	void createNewModel();
	void initializeModelFromPsd(const char* fileName);

	void saveModel();
	int loadModel(const char* filePath);

	bool isRunning = true;
private:
	void checkRunning();
};

