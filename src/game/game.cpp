#include "game.h"
#include "utils.h"

Window::Window(int32_t width, int32_t height, const char* title)
	: width(width), height(height), title(title)
{
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

Window::Window()
{
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void Window::update()
{
	glClear(GL_COLOR_BUFFER_BIT);
	swapBuffers();
	glfwPollEvents();
}

Window* Game::mWindow;
GameManager* Game::mManager;
float Game::delta;
float Game::lastTime;

void Game::Run()
{
	init();
	loop();
	cleanup();
}

void Game::init()
{
	ASSERT(glfwInit() == GLFW_TRUE, "failed to initialize glfw!");

	mWindow = new Window();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ASSERT(glewInit() == GLEW_OK, "failed to initialize glew!");

	mManager = new GameManager();

}

void Game::cleanup()
{
	delete mManager;
	delete mWindow;
}

void Game::loop()
{
	delta = 0.0f;
	lastTime = (float)glfwGetTime();
	while (!mWindow->shouldClose())
	{
		delta = (float)glfwGetTime() - lastTime;
		lastTime = (float)glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT);
		mManager->render(delta);
		mManager->update(delta);
		glfwSwapBuffers(mWindow->window);
		glfwPollEvents();
		//mWindow->update();
	}
}