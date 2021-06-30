#include "Application.h"

Application::Application()
{
	init();
}

Application::~Application()
{
}

void Application::update()
{
	window->update();
	model->update();
	//model->renderModel();

	glfwSwapBuffers(window->getWindow());
	glfwPollEvents();

	checkRunning();
}

void Application::createNewModel()
{
	model = std::make_unique<Model>();
}

void Application::initializeModelFromPsd(const char* fileName)
{
	TextureLoader::loadPsdFile(fileName);
}

void Application::addModelMesh(const char* filePath)
{
	model->addMesh(filePath);
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
	window = std::make_unique<Window>();

	glfwSetFramebufferSizeCallback(window->getWindow(), Event::framebuffer_size_callback);

	stbi_set_flip_vertically_on_load(true);
}

void Application::checkRunning()
{
	if (glfwWindowShouldClose(window->getWindow()))
	{
		isRunning = false;
	}
}
