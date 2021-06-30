#include "Window.h"

Window::Window()
{
	init();
}

Window::~Window()
{
	glfwTerminate();
}

void Window::update()
{
	Event::processInput(m_window);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//glfwSwapBuffers(m_window);
	//glfwPollEvents();
}

GLFWwindow* Window::getWindow()
{
	return m_window;
}

void Window::init()
{
	//setup glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//other settings
	//glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	//create a new window
	m_window = glfwCreateWindow(800, 450, "Test Window", NULL, NULL);
	if (m_window == NULL)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_window);

	//glad thing
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return;
	}

	//does some transparency things idk
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//if 3D, prob not
	//glEnable(GL_DEPTH_TEST);

	//sets up vsync
	glfwSwapInterval(1);

	//only render front side of meshes
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	//draw in lines, testing triangulation
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//hope nothing exploded
	std::cout << "Window creation successful\n";
}
