#pragma once
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <chrono>

#include "game_manager.h"

struct Window
{
	int32_t width = 800, height = 600;
	const char* title = "Pacman";
	GLFWwindow* window;

	Window(int32_t width, int32_t height, const char* title);
	Window();
	~Window();
	void init();

	void update();

	inline bool shouldClose() const { return glfwWindowShouldClose(window); }
	inline void swapBuffers() const { glfwSwapBuffers(window); }
};

class Game
{
public:
	static void Run();

	static bool locked;
private:
	static void init();
	static void cleanup();
	static void loop();

private:
	static Window* mWindow;
	static GameManager* mManager;
	static float delta;
	static std::chrono::steady_clock::time_point cur_time;
	static std::chrono::steady_clock::time_point last_time;
};