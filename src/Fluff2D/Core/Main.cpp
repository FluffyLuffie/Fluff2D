#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Application.h"

/*
in project properties, C/C++ -> Code Generation -> Runtime Libraries
might need to do cmake stuff to use non debug dll (Md)
https://discourse.glfw.org/t/setting-up-glfw-on-visual-studio/1344/3
*/

int main()
{
	Application application;
	application.init();

	//framebuffer fails for some reason, open through the file system
	//application.initializeModelFromPsd("saves/tempPsdTest/miniTest.psd");
	//application.initializeModelFromPsd("saves/tempPsdTest/testModel.psd");
	//application.initializeModelFromPsd("saves/tempPsdTest/edwardGraveheart.psd");
	//application.initializeModelFromPsd("saves/tempPsdTest/fluffy.psd");
	//application.initializeModelFromPsd("saves/tempPsdTest/boxTest.psd");
	//application.initializeModelFromPsd("saves/tempPsdTest/alya.psd");

	while (application.isRunning)
	{
		application.update();
	}

	return 0;
}