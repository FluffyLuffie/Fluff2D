#include "Window.h"


Window::Window()
{
}

Window::~Window()
{
	glfwTerminate();
}

void Window::update()
{
	glClearColor(Settings::backgroundColor.r, Settings::backgroundColor.g, Settings::backgroundColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

GLFWwindow* Window::getWindow()
{
	return m_window;
}

void Window::init()
{
	//setup glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//for transparent background
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

	//other settings
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	//create a new window
	m_window = glfwCreateWindow(800, 450, "Fluff2D", NULL, NULL);
	if (m_window == NULL)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_window);
	glfwGetFramebufferSize(m_window, &width, &height);

	//glad thing
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return;
	}

	//window icon
	GLFWimage data[1];
	int junk = 0;
	data[0].pixels = stbi_load("resources/textures/icon.png", &data[0].width, &data[0].height, &junk, 0);
	glfwSetWindowIcon(m_window, 1, data);
	stbi_image_free(data[0].pixels);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	//if 3D, prob not
	//glEnable(GL_DEPTH_TEST);

	//sets up vsync
	glfwSwapInterval(1);

	//only render front side of meshes
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	//draw in lines, testing triangulation
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//testing drawing points
	glEnable(GL_PROGRAM_POINT_SIZE);

	//hope nothing exploded
	Log::info("Window creation successful");
}
