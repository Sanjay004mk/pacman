#pragma once

#define  GRID_WIDTH			32;
#define  GRID_HEIGHT		32;
#define TEXTURE_DIVS_U		1.0f;
#define TEXTURE_DIVS_V		1.0f;

#include <GL/glew.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "cell.h"
#include "utils.h"

class GameManager
{
public:
	GameManager();
	~GameManager();

	void render(float delta);
	void update(float delta);

private:
	/*
	std::vector<std::vector<Cell*>> mCells;
	std::vector<std::vector<glm::vec2>> mCellTexOffsets;
	std::vector<std::vector<glm::vec2>> mCellPosOffsets;
	*/

	uint32_t program;
	uint32_t vao;
	uint32_t ibo;
	uint32_t vbo;
};