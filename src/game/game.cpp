#include "game.h"
#include "data.h"


/*/// forward declarations ///*/
void windowResizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

////////////////////////////////////////
/// struct: Window /////////////////////
////////////////////////////////////////

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

/* create window, set minimum window size and callback functions */
void Window::init()
{
#ifdef GAME_DEBUG
	showConsoleWindow();
#else
	hideConsoleWindow();
#endif
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	/* +3 is to accomodate for ui elements */
	glfwSetWindowSizeLimits(window,(int)(400.0f * ((float)GRID_WIDTH / (float)(GRID_HEIGHT + 3))), 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
}

void Window::update()
{
	swapBuffers();
	glfwPollEvents();
}

//////////////////////////////////////
/// class: Game //////////////////////
//////////////////////////////////////

Window* Game::mWindow;
GameManager* Game::mManager;
float Game::delta;
std::chrono::steady_clock::time_point Game::cur_time;
std::chrono::steady_clock::time_point Game::last_time;
bool Game::locked = false;

void Game::Run()
{
	init();
	loop();
	cleanup();
}

void Game::init()
{
	ASSERT(glfwInit() == GLFW_TRUE, "[GLFW_ERROR]: failed to initialize glfw!");

	mWindow = new Window();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ASSERT(glewInit() == GLEW_OK, "[GLEW_ERROR]: failed to initialize glew!");

	AudioManager::init();
	mManager = new GameManager((float)mWindow->width / (float)mWindow->height);
	mManager->setWindowPtr(mWindow);
	glfwSetWindowUserPointer(mWindow->window, mManager);
	GLFWimage icon;
	icon.width = 32;
	icon.height = 32;
	(uint8_t*)icon.pixels = sIcon.data();
	glfwSetWindowIcon(mWindow->window, 1, &icon);
}

void Game::cleanup()
{
	AudioManager::shutDown();

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
		delta = fmodf(std::chrono::duration<float>(cur_time - last_time).count(), 1.0f);
		last_time = std::chrono::high_resolution_clock::now();
		//LOG_FPS(delta);

		glClear(GL_COLOR_BUFFER_BIT);
		mManager->render(delta);
		mManager->update(delta);
		mWindow->update();
	}
}

/*/// callback functions ///*/

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	if (Game::locked)
		return;

	Game::locked = true;

	GameManager* m = (GameManager*)glfwGetWindowUserPointer(window);
	m->mWindow->width = width;
	m->mWindow->height = height;
	float aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
	m->setMatrix(aspectRatio);

	Game::locked = false;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (Game::locked)
		return;

	Game::locked = true;
	Game::locked = false;

	if (action != GLFW_PRESS)
		return;

	GameManager* m = (GameManager*)glfwGetWindowUserPointer(window);
	float x = 0.0f, y = 0.0f;
	if (key == GLFW_KEY_UP)
	{
		y = 1.0f;
	}
	else if (key == GLFW_KEY_DOWN)
	{
		y = -1.0f;
	}
	else if (key == GLFW_KEY_LEFT)
	{
		x = -1.0f;
	}
	else if (key == GLFW_KEY_RIGHT)
	{
		x = 1.0f;
	}

	m->pacman->direction[NEW_DIR] = { x, y };
	
	Game::locked = false;
}