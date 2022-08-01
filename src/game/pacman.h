#pragma once
#include "utils.h"

struct GLFWwindow;

#define CUR_DIR 0
#define NEW_DIR 1

void addPoints(int32_t points);
int32_t getPoints();

class Pacman
{
public:
	Pacman(const glm::mat4& projMatrix, uint32_t pacmanTexture);
	~Pacman();
	void render(float delta);
	int32_t move(float delta, eCellType* grid, bool& shouldUpdate);

	inline void updateMatrix(const glm::mat4& matrix) { 
		projMatrix = matrix; 
		glUseProgram(pacmanShaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(pacmanShaderProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
		glUseProgram(0);
	}

private:
	void init();

	glm::vec2 position;
	std::array<glm::vec2, 2> direction;
	glm::mat4 projMatrix;

	bool poweredUp = false;

	float acc;
	float moveSpeed = 4.0f;
	float difficulty = 1.0f;
	float moveSpeedMod = 1.0f;

	Quad sprite;
	QuadIndices indices;
	glm::mat4 modelMatrix;
	uint32_t frame; //not used...... yet

	uint32_t pacmanShaderProgram;
	uint32_t vao;
	uint32_t vbo;
	uint32_t ibo;
	std::array<int32_t, 2> animFrame;
	uint32_t texture;


	friend class GameManager;
	friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};