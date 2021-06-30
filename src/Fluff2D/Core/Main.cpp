#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Fluff2D/Core/Application.h"

int main()
{
	std::unique_ptr<Application> application = std::make_unique<Application>();

	application->createNewModel();

	//testing
	application->addModelMesh("saves/textures/chibiFluffy.png");

	//testing loading psd
	//application->initializeModelFromPsd("saves/tempPsdTest/miniTest.psd");
	application->initializeModelFromPsd("saves/tempPsdTest/testModel.psd");
	//application->initializeModelFromPsd("saves/tempPsdTest/edwardGraveheart.psd");
	//application->initializeModelFromPsd("saves/tempPsdTest/fluffy.psd");
	
	//don't call, yet
	//application->loadModel("saves/textures/chibiFluffy.png");

	while (application->isRunning)
	{
		application->update();
	}

	return 0;
}