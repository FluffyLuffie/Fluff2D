#pragma once

#include <memory>
#include "Fluff2D/Core/Window.h"
#include "Fluff2D/Events/Event.h"
#include "Fluff2D/Renderer/Model.h"
#include "Fluff2D/Renderer/ModelMesh.h"

class Application
{
public:
	Application();
	~Application();

	void update();

	void createNewModel();
	void initializeModelFromPsd(const char* fileName);
	void addModelMesh(const char* filePath);
	int loadModel(const char* filePath);

	bool isRunning = true;
private:
	std::unique_ptr<Window> window;
	std::unique_ptr<Model> model;

	void init();
	void checkRunning();
};

