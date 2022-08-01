#include "ghosts.h"
#include "audio_manager.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "data.h"

////////////////////////////////////
/// class: Ghost ///////////////////
////////////////////////////////////

Ghost::Ghost(eCellType* cells, GhostManager* manager, int32_t ghostID, float difficulty)
	: direction({ 0.0f, 0.0f }), prevDir({ 0.0f, 0.0f }), position({ 15.0f, 16.0f }), target({ 14.0f, 17.0f }), mode(SCATTER), modeChanged(false), manager(manager), ghostID(ghostID), difficulty(difficulty)
{
	pathFind(cells);
	/* add delay to certain ghosts before they start moving */
	switch (ghostID)
	{
	case BLINKY:
		startTime = 0.0f;
		break;
	case INKY:
		position.x--;
		startTime = 3.0f;
		break;
	case PINKY:
		position.x -= 2.0f;
		startTime = 5.0f;
		break;
	case CLYDE:
		position.x -= 3.0f;
		startTime = 9.0f;
		break;
	}
}

Ghost::~Ghost()
{

}

void Ghost::update(float delta, eCellType* cells, const glm::vec2& pacmanPos, const glm::vec2& pacmanDir)
{
	acc += delta, modeChangeTimer += delta, timer += delta;

	/* wait for certain amount of time before starting to move */
	if (timer < startTime)
		return;

	/* change mode to scatter or chase based on how long it has been in it's current mode */
	if (modeChangeTimer > (SCATTER_TIME / difficulty) && (mode == SCATTER || mode == SCARED))
	{
		modeChangeTimer = 0.0f;
		mode = CHASE;
		modeChanged = true;
	}
	else if (modeChangeTimer > (CHASE_TIME * difficulty))
	{
		modeChangeTimer = 0.0f;
		mode = SCATTER;
		modeChanged = true;
	}

	/* set target cell based on ghost and mode */
	if (mode == CHASE)
	{
		switch (ghostID)
		{
		case BLINKY:
			target = pacmanPos;
			break;
			
		case INKY:
			target = getNearestValidTarget(manager->getGhostPosition(BLINKY) + (manager->getGhostTarget(PINKY) - manager->getGhostPosition(BLINKY)) * 2.0f, cells, pacmanPos);
			break;

		case PINKY:
			target = getNearestValidTarget(pacmanDir * 2.0f + pacmanPos, cells, pacmanPos);
			break;

		case CLYDE:
			target = pacmanPos;
			break;
		}
	}
	else if (mode == SCATTER)
	{
		switch (ghostID)
		{
		case BLINKY:
			target = glm::vec2(GRID_WIDTH - 2.0f, GRID_HEIGHT - 2.0f);
			break;
		case INKY:
			target = glm::vec2(GRID_WIDTH - 2.0f, 1.0f);
			break;

		case PINKY:
			target = glm::vec2(1.0f, GRID_HEIGHT - 2.0f);
			break;

		case CLYDE:
			target = glm::vec2(1.0f, 1.0f);
			break;
		}
	}

	if (acc * moveSpeed * difficulty > 1.0f)
	{
		acc = 0.0f;
		move(cells);
	}

	/* re-calculate path every 3 seconds */
	static float timer = 0.0f;
	if (timer += delta > 3.0f)
	{
		timer = 0.0f;
		pathFind(cells);
	}
}

void Ghost::move(eCellType* cells)
{	
	if (movesToTarget.empty())
	{
		pathFind(cells);
		return;
	}

	/* move in the direction of the latest GhostMove and decrement the counter for the GhostMove, 
	   if counter reaches zero remove it from vector and find a new path */
	direction = movesToTarget.back().first;
	position += direction;
	if (--movesToTarget.back().second <= 0)
	{
		movesToTarget.pop_back();
		pathFind(cells);
		if (!movesToTarget.empty())
			direction = movesToTarget.back().first;
	}
	/* if ghost has reached target set direction to zero to prevent animation glitch and also reset movement speed */
	if (position == target)
	{
		direction = glm::vec2(0.0f);
		mode = mode == SCARED ? CHASE : -mode;
		moveSpeed = 4.0f * difficulty;
	}
}

void Ghost::pathFind(eCellType* grid)
{
	std::unordered_map<glm::vec2, glm::vec2> cameFrom;
	std::vector<glm::vec2> available;
	movesToTarget.clear();
	available.push_back(position);
	cameFrom.insert(std::pair<glm::vec2, glm::vec2>(position, position));

	/* keep pathfinding as long as there are tiles that aren't visited or the target tile hasn't been reached,
	   whichever comes first */
	while (!available.empty())
	{
		/* take the oldest tile and remove it from the vector */
		glm::vec2 node = available.front();	
		available.erase(available.begin());

		if (node == target)
			break;

		/* all possible neighbouring tiles */
		glm::vec2 next[] = {
			{ node.x, node.y + 1.0f},
			{ node.x, node.y - 1.0f},
			{ node.x + 1.0f, node.y},
			{ node.x - 1.0f, node.y}
		};

		for (size_t i = 0; i < 4; i++)
		{
			/* if n doesn't exist in the unordered_map and the grid cell isn't a wall,
			   add it to available nodes and set cameFrom */
			if (!cameFrom.count(next[i]) && !(grid[(int32_t)next[i].x + (int32_t)next[i].y * GRID_WIDTH] & eCellType::WALL))
			{
				cameFrom.insert(std::pair<glm::vec2, glm::vec2>(next[i], node));
				available.push_back(next[i]);
			}
		}
	}

	/* create a path for ghosts to navigate from pathfinding data */
	glm::vec2 node = target;
	glm::vec2 dir = node - cameFrom[node];
	int32_t steps = 1;
	while (node != position)
	{
		/* cameFrom doesn't contain a key value matching node, so it constructs a new key and it's value is glm::vec2() */
		if ((node = cameFrom[node]) == glm::vec2())
		{
			GAME_LOG("unable to find path to target!");
			direction = glm::vec2(0.0f);
			break;
		}
		if (dir == node - cameFrom[node])
			steps++;
		else
		{
			movesToTarget.push_back(std::pair<glm::vec2, int32_t>(dir, steps));
			dir = node - cameFrom[node];
			steps = 1;
		}
	}
}

/* doesn't work */
glm::vec2 Ghost::getNearestValidTarget(const glm::vec2& target, eCellType* grid, const glm::vec2& pacmanPos)
{
	glm::vec2 res = { std::min((float)(GRID_WIDTH - 2), std::max(target.x, 1.0f)), std::min((float)(GRID_HEIGHT - 2), std::max(target.y, 1.0f)) };
	glm::vec2 next[] = {
		{ res.x, res.y + 1.0f},
		{ res.x, res.y - 1.0f},
		{ res.x + 1.0f, res.y},
		{ res.x - 1.0f, res.y},
		{ res.x - 1.0f, res.y - 1.0f},
		{ res.x - 1.0f, res.y + 1.0f},
		{ res.x + 1.0f, res.y - 1.0f},
		{ res.x + 1.0f, res.y + 1.0f}
	};

	for (size_t i = 0; i < 8; i++)
	{
		if (!(grid[(int32_t)(next[i].x) + (int32_t)(next[i].y) * GRID_WIDTH] & eCellType::WALL))
			return next[i];
	}

	return pacmanPos;
}

///////////////////////////////////
/// class: GhostManager ///////////
///////////////////////////////////

GhostManager::GhostManager(const glm::mat4& projMatrix, eCellType* grid)
	: projMatrix(projMatrix)
{
	for (size_t i = 0; i < 4; i++)
		ghosts[i] = new Ghost(grid, this, (int32_t)i, 1.0f);

	init();
}

GhostManager::~GhostManager()
{
	for (auto& ghost : ghosts)
		delete ghost;

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(ghostProgram);
}

void GhostManager::init()
{
	for (auto& sprite : sprites)
	{
		sprite = Quad({ 0.0f, 0.0f }, { 1.0f, 1.0f });
		sprite.vertices[0].texCoords = glm::vec2(0.0f + (0.125f / 32.0f), 0.25f);
		sprite.vertices[1].texCoords = glm::vec2(0.25f - (0.125f / 32.0f), 0.25f);
		sprite.vertices[2].texCoords = glm::vec2(0.25f - (0.125f / 32.0f), 0.0f);
		sprite.vertices[3].texCoords = glm::vec2(0.0f + (0.125f / 32.0f), 0.0f);
	}

	for (size_t i = 0; i < indices.size(); i++)
	{
		indices[i].setQuadIndices((uint32_t)(i * 4));
		ghostModelMatrix[i] = glm::translate(glm::mat4(1.0f), glm::vec3(ghosts[i]->position.x, ghosts[i]->position.y, 0.0f));
	}



	ghostProgram = glCreateProgram();
	std::string vSource = sShaders[GHOST_SHADER].vertex, fSource = sShaders[GHOST_SHADER].fragment;
	const char* vShader = vSource.c_str(), * fShader = fSource.c_str();

	uint32_t vs = createShader(GL_VERTEX_SHADER, vShader);
	uint32_t fs = createShader(GL_FRAGMENT_SHADER, fShader);

	glAttachShader(ghostProgram, vs);
	glAttachShader(ghostProgram, fs);
	glLinkProgram(ghostProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);

	/* set opengl uniform */
	updateMatrix(projMatrix);	

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad) * sprites.size(), sprites.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(QuadIndices), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	loadTexture();
}

void GhostManager::loadTexture()
{
	int32_t width = sTextures[GHOST_TEXTURE].width, height = sTextures[GHOST_TEXTURE].height, channels = sTextures[GHOST_TEXTURE].channels;
	const uint8_t* data = sTextures[GHOST_TEXTURE].data.data();

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GhostManager::setDifficulty(float difficulty)
{
	for (auto& ghost : ghosts)
		ghost->difficulty = difficulty;
}

void GhostManager::render()
{
	glUseProgram(ghostProgram);
	glUniformMatrix4fv(glGetUniformLocation(ghostProgram, "model"), 4, GL_FALSE, (float*)ghostModelMatrix.data());
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glUniform2f(glGetUniformLocation(ghostProgram, "direction"), ghosts[0]->direction.x, ghosts[0]->direction.y);
	glUniform1i(glGetUniformLocation(ghostProgram, "ghostID"), 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glUniform2f(glGetUniformLocation(ghostProgram, "direction"), ghosts[1]->direction.x, ghosts[1]->direction.y);
	glUniform1i(glGetUniformLocation(ghostProgram, "ghostID"), 1);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glUniform2f(glGetUniformLocation(ghostProgram, "direction"), ghosts[2]->direction.x, ghosts[2]->direction.y);
	glUniform1i(glGetUniformLocation(ghostProgram, "ghostID"), 2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glUniform2f(glGetUniformLocation(ghostProgram, "direction"), ghosts[3]->direction.x, ghosts[3]->direction.y);
	glUniform1i(glGetUniformLocation(ghostProgram, "ghostID"), 3);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

bool GhostManager::ghostCaughtPacman(float delta, eCellType* cells, bool& ateGhost, const glm::vec2& pacmanPos, const glm::vec2& pacmanDir, bool isPacmanPoweredUp)
{
	for (size_t i = 0; i < ghosts.size(); i++)
	{
		ghosts[i]->update(delta, cells, pacmanPos, pacmanDir);
		ghostModelMatrix[i] = glm::translate(glm::mat4(1.0f), glm::vec3(ghosts[i]->position.x, ghosts[i]->position.y, 0.0f) + glm::vec3(ghosts[i]->direction.x, ghosts[i]->direction.y, 0.0f) * ghosts[i]->acc * ghosts[i]->moveSpeed * ghosts[i]->difficulty);

		static bool immune = false;
		static float timer = 0.0f;
		if (immune)
		{
			timer += delta;
			if (timer > 1.0f)
			{
				timer = 0.0f;
				immune = false;
			}
		}
		if (ghosts[i]->position == pacmanPos && ghosts[i]->mode != SCARED)
		{
			
			if (isPacmanPoweredUp)
			{
				/* pacman eats ghost and gains one second immunity */
				ghosts[i]->target = { 14.0f, 16.0f };
				ghosts[i]->modeChangeTimer = 0.0f;
				ghosts[i]->mode = SCARED;
				ghosts[i]->pathFind(cells);
				ghosts[i]->moveSpeed = 9.0f;
				immune = true;
				ateGhost = true;
				AudioManager::playSound(PACMAN_EAT_GHOST);
			}
			else if (!immune)
			{
					return true;
			}

		}
	}

	return false;
}