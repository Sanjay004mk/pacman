#pragma once

constexpr size_t GRID_WIDTH			= 32;
constexpr size_t GRID_HEIGHT		= 32;
constexpr float TEXTURE_DIVS_U		= 1.0f;
constexpr float TEXTURE_DIVS_V		= 1.0f;

#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils.h"

enum class eCellType {
	EMPTY		=	0,
	WALL		=	1,
	WALL_TOP	=	((1 << 1) | 1),
	WALL_BOTTOM =	((1 << 2) | 1),
	WALL_LEFT	=	((1 << 3) | 1),
	WALL_RIGHT	=	((1 << 4) | 1),
	FRUIT0		=	(1 << 5),
	FRUIT1		=	(1 << 6),
	FRUIT2		=	(1 << 7)
};

struct Vertex
{
	glm::vec2 position;
	glm::vec2 texCoords;
};

struct Quad
{
	std::array<Vertex, 4> vertices;
};

struct QuadIndices
{
	std::array<uint32_t, 6> indices;
	void setQuadIndices(uint32_t index)
	{
		indices[0] = index + 0;
		indices[1] = index + 1;
		indices[2] = index + 2;
		indices[3] = index + 2;
		indices[4] = index + 3;
		indices[5] = index + 0;
	}
};

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

private:
	void init();

	std::array<std::array<eCellType, GRID_HEIGHT>, GRID_WIDTH> cellTypes;
	std::array<std::array<Quad, GRID_HEIGHT>, GRID_WIDTH> cells;
	std::array<std::array<QuadIndices, GRID_HEIGHT>, GRID_WIDTH> cellsIndices;

	glm::mat4 projMatrix;

	uint32_t cellShaderProgram;
	uint32_t vao;
	uint32_t ibo;
	uint32_t vbo;
	Window* mWindow;

	friend void windowResizeCallback(GLFWwindow* window, int width, int height);
};