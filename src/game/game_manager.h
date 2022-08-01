#pragma once

#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include "audio_manager.h"
#include "pacman.h"
#include "ghosts.h"


struct Window;

class GameManager
{
public:
	GameManager(float aspectRatio);
	~GameManager();

	void render(float delta);
	void update(float delta);

	void setMatrix(float aspectRatio);
	void setWindowPtr(Window* w) { mWindow = w; }

	void resetLevel();
	void resetPacmanAndGhosts();
private:
	void init();
	void initUI();
	void updateUI();
	void loadmap();
	void loadWallTexture();
	void loadPacmanTexture();
	void loadNumTexture();
	void genBuffers();
	void deleteBuffers();
	void updateBuffers();
	void gameReset();
	void gameWon();
	void gameOver();
	int32_t getFruitCount();

	eCellType* cellTypes = nullptr;

	uint32_t cellShaderProgram;
	std::array<std::array<Quad, GRID_HEIGHT>, GRID_WIDTH> cells;
	std::array<std::array<QuadIndices, GRID_HEIGHT>, GRID_WIDTH> cellsIndices;
	uint32_t vao;
	uint32_t ibo;
	uint32_t vbo;
	uint32_t wallTexture;
	int32_t* animFrame;

	glm::mat4 projMatrix;

	Pacman* pacman = nullptr;
	uint32_t pacmanLives = 3;
	int32_t points = 0;

	uint32_t uiProgram;
	uint32_t numTexture;
	uint32_t scoreTexture;
	uint32_t pacmanTexture;
	
	std::array<Quad, 3> lives;
	std::array<QuadIndices, 3> livesIndices;
	uint32_t lvao;
	uint32_t libo;
	uint32_t lvbo;
	
	std::array<Quad, 5> pointsNum;
	std::array<QuadIndices, 5> pointsNumIndices;
	uint32_t pvao;
	uint32_t pibo;
	uint32_t pvbo;
	
	GhostManager* ghostManager = nullptr;
	bool gameStart = true;

	Window* mWindow = nullptr;

	friend void windowResizeCallback(GLFWwindow* window, int width, int height);
	friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};