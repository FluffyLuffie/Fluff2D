#include "Application.h"

Application::Application()
{
}

Application::~Application()
{
}

void Application::update()
{
	window.update();
	model.update();
	model.render();

	glfwSwapBuffers(window.getWindow());
	glfwPollEvents();

	Event::calculateDeltaTime();

	checkRunning();
}

void Application::createNewModel()
{
	//model = Model();
}

void Application::initializeModelFromPsd(const char* fileName)
{
	TextureLoader::loadPsdFile(fileName, &model);
	TextureLoader::loadTexture(&model.textureID, "saves/testExports/textureAtlas.png", &model.atlasWidth, &model.atlasHeight, &model.atlasNrChannels);
	model.updateMeshNumber();
}

void Application::saveModel()
{
	SaveSystem::saveModel(model);
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

	model.setShader();

	glfwSetFramebufferSizeCallback(window.getWindow(), Event::framebuffer_size_callback);
	glfwSetScrollCallback(window.getWindow(), Event::scroll_callback);

	stbi_set_flip_vertically_on_load(true);
}

void Application::checkRunning()
{
	if (glfwWindowShouldClose(window.getWindow()))
	{
		isRunning = false;
	}
}
