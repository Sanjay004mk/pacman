#pragma once
#include "utils.h"
#include <unordered_map>
#include <glm/gtx/hash.hpp>

#define BLINKY			0
#define INKY			1
#define PINKY			2
#define CLYDE			3

#define CHASE			1
#define SCARED			0
#define SCATTER		   -1

#define SCATTER_TIME	7.0f
#define CHASE_TIME		20.0f

using GhostMove = std::pair<glm::vec2, int32_t>;
using GhostMode = int32_t;

class GhostManager;

class Ghost
{
public:
	Ghost(eCellType* grid, GhostManager* manager, int32_t ghostID, float difficulty);
	~Ghost();

	void update(float delta, eCellType* cells, const glm::vec2& pacmanPos, const glm::vec2& pacmanDir);
private:
	void move(eCellType* cells);
	void pathFind(eCellType* grid);
	glm::vec2 getNearestValidTarget(const glm::vec2& target, eCellType* grid, const glm::vec2& pacmanPos);

	int32_t ghostID;
	glm::vec2 position;
	glm::vec2 direction;
	glm::vec2 prevDir;
	glm::vec2 target;
	std::vector<GhostMove> movesToTarget;
	GhostMode mode;
	bool modeChanged;

	float startTime = 0.0f;
	float timer = 0.0f;
	bool started = false;
	
	float acc = 0.0f;
	float modeChangeTimer = 0.0f;
	float moveSpeed = 4.0f;
	float difficulty = 1.0f;

	GhostManager* manager;

	friend class GameManager;
	friend class GhostManager;
};

class GhostManager
{
public:
	GhostManager(const glm::mat4& projMatrix, eCellType* grid);
	~GhostManager();

	void render();
	bool ghostCaughtPacman(float delta, eCellType* cells, bool& ateGhost, const glm::vec2& pacmanPos, const glm::vec2& pacmanDir, bool isPacmanPoweredUp);

	inline void updateMatrix(const glm::mat4& projMatrix)
	{ 
		this->projMatrix = projMatrix;
		glUseProgram(ghostProgram);
		glUniformMatrix4fv(glGetUniformLocation(ghostProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
		glUseProgram(0);
	}

	inline const glm::vec2& getGhostPosition(int32_t ghostID) const { return ghosts[ghostID]->position; }
	inline const glm::vec2& getGhostTarget(int32_t ghostID) const { return ghosts[ghostID]->target; }

	void setDifficulty(float difficulty);
private:
	void init();
	void loadTexture();

	std::array<Ghost*, 4> ghosts;

	std::array<Quad, 4> sprites;
	std::array<QuadIndices, 4> indices;

	glm::mat4 projMatrix;
	std::array<glm::mat4, 4> ghostModelMatrix;

	uint32_t ghostProgram;
	uint32_t ibo;
	uint32_t vao;
	uint32_t vbo;
	uint32_t texture;

	friend class GameManager;
};