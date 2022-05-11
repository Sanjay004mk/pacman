#include "game.h"
#include "utils.h"

void windowResizeCallback(GLFWwindow* window, int width, int height);

Window::Window(int32_t width, int32_t height, const char* title)
	: width(width), height(height), title(title)
{
	init();
}

Window::Window()
{
	init();
}

Window::~Window()
{
	glfwDestroyWindow(window);
}

void Window::init()
{
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
}

void Window::update()
{
	swapBuffers();
	glfwPollEvents();
}

Window* Game::mWindow;
GameManager* Game::mManager;
float Game::delta;
std::chrono::steady_clock::time_point Game::cur_time;
std::chrono::steady_clock::time_point Game::last_time;

void Game::Run()
{
	init();
	loop();
	cleanup();
}

void Game::init()
{
	ASSERT(glfwInit() == GLFW_TRUE, "[GLFW]: ERROR: failed to initialize glfw!");

	mWindow = new Window();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ASSERT(glewInit() == GLEW_OK, "[GLEW]: ERROR: failed to initialize glew!");

	mManager = new GameManager((float)mWindow->width / (float)mWindow->height);
	mManager->setWindowPtr(mWindow);
	glfwSetWindowUserPointer(mWindow->window, mManager);
}

void Game::cleanup()
{
	delete mManager;
	delete mWindow;
}

void Game::loop()
{
	delta = 0.0f;
	cur_time = last_time = std::chrono::high_resolution_clock::now();
	while (!mWindow->shouldClose())
	{
		cur_time = std::chrono::high_resolution_clock::now();
		delta = std::chrono::duration<float>(cur_time - last_time).count();
		last_time = std::chrono::high_resolution_clock::now();

		mManager->render(delta);
		mManager->update(delta);
		mWindow->update();
	}
}

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	GameManager* m = (GameManager*)glfwGetWindowUserPointer(window);
	m->mWindow->width = width;
	m->mWindow->height = height;
	float aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
	m->setMatrix(aspectRatio);
}
