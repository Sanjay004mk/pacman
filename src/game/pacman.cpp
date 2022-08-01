#include "pacman.h"
#include "audio_manager.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include "data.h"

///////////////////////////////////////
/// class: Pacman /////////////////////
///////////////////////////////////////

Pacman::Pacman(const glm::mat4& projMatrix, uint32_t pacmanTexture)
	: projMatrix(projMatrix), modelMatrix(1.0f), frame(0), moveSpeed(4.0f), acc(0.0f), position({ 14.0f, 7.0f }), texture(pacmanTexture)
{

	direction[CUR_DIR] = { 1.0, 0.0 };
	direction[NEW_DIR] = { 0.0, 0.0 };
	init();
}

Pacman::~Pacman()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(pacmanShaderProgram);
}

void Pacman::init()
{
	sprite = Quad({ 0.0f, 0.0f }, { 1.0f, 1.0f });
	sprite.vertices[0].texCoords = glm::vec2(0.0f + (0.125f / 32.0f), 0.0f);
	sprite.vertices[1].texCoords = glm::vec2(0.125f - (0.125f / 32.0f), 0.0f);
	sprite.vertices[2].texCoords = glm::vec2(0.125f - (0.125f / 32.0f), 1.0f);
	sprite.vertices[3].texCoords = glm::vec2(0.0f + (0.125f / 32.0f), 1.0f);
	indices.setQuadIndices(0);

	animFrame[0] = 0;
	animFrame[1] = 0;

	/* opengl stuff */
	pacmanShaderProgram = glCreateProgram();
	std::string vSource = sShaders[PACMAN_SHADER].vertex, fSource = sShaders[PACMAN_SHADER].fragment;
	const char* vShader = vSource.c_str(), * fShader = fSource.c_str();

	uint32_t vertexShader = createShader(GL_VERTEX_SHADER, vShader);
	uint32_t fragmentShader = createShader(GL_FRAGMENT_SHADER, fShader);

	glAttachShader(pacmanShaderProgram, vertexShader);
	glAttachShader(pacmanShaderProgram, fragmentShader);
	glLinkProgram(pacmanShaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(pacmanShaderProgram);
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(pacmanShaderProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(pacmanShaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniform1iv(glGetUniformLocation(pacmanShaderProgram, "animFrame"), 2, animFrame.data());
	glUseProgram(0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite), sprite.vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Pacman::render(float delta)
{
	glUseProgram(pacmanShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(pacmanShaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

int32_t Pacman::move(float delta, eCellType* grid, bool& shouldUpdate)
{
	int32_t points = 0;
	/* get coordinates of next cell in pacman's current direction */
	size_t x = (size_t)(std::floor(position.x) + direction[CUR_DIR].x) % GRID_WIDTH;
	size_t y = (size_t)(std::floor(position.y) + direction[CUR_DIR].y) % GRID_HEIGHT;

	/* if there is any input from the player to change direction, check if the cell in the new direction is not a wall and change 
	   direction if pacman just moved (acc == 0.0f) */
	if (direction[CUR_DIR] != direction[NEW_DIR] && direction[NEW_DIR] != glm::vec2(0.0f))
	{
		size_t alt_x = (size_t)(position.x + direction[NEW_DIR].x) % GRID_WIDTH;
		size_t alt_y = (size_t)(position.y + direction[NEW_DIR].y) % GRID_HEIGHT;

		if (!(grid[alt_x + alt_y * GRID_WIDTH] & eCellType::WALL) && acc == 0.0f)
		{
			x = alt_x;
			y = alt_y;
			direction[CUR_DIR] = direction[NEW_DIR];
			direction[NEW_DIR] = { 0.0f, 0.0f };
		}
	}

	static float animTimer = 0.0f;
	if ((animTimer += delta) > (1.0f / 4.0f))
	{
		animTimer = 0.0f;
		animFrame[0] = ++animFrame[0] % 3;
		animFrame[1] = (rand() % 4 > 1) ? 0 : 1;
		glUseProgram(pacmanShaderProgram);
		glUniform1iv(glGetUniformLocation(pacmanShaderProgram, "animFrame"), 2, animFrame.data());
		glUseProgram(0);
	}

	if (!(grid[x + y * GRID_WIDTH] & eCellType::WALL))
	{
		if (grid[x + y * GRID_WIDTH] & eCellType::TELEPORT)
		{
			position.x = (position.x == 1.0f) ? (float)(GRID_WIDTH - 2) : 1.0f;
			return 0;
		}
		acc += delta;
		if (acc * moveSpeed * moveSpeedMod * difficulty > 1.0f)
		{
			acc = 0.0f;
			moveSpeedMod = 1.2f;
			position += direction[CUR_DIR];
		}

		if (direction[CUR_DIR].x > 0.0f)
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f) + glm::vec3(direction[CUR_DIR].x, direction[CUR_DIR].y, 0.0f) * acc * moveSpeed * moveSpeedMod * difficulty);

		else if (direction[CUR_DIR].y > 0.0f)
		{
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x + 1.0f, position.y, 0.0f) + glm::vec3(direction[CUR_DIR].x, direction[CUR_DIR].y, 0.0f) * acc * moveSpeed * moveSpeedMod * difficulty);

			modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (direction[CUR_DIR].y < 0.0f)
		{
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y + 1.0f, 0.0f) + glm::vec3(direction[CUR_DIR].x, direction[CUR_DIR].y, 0.0f) * acc * moveSpeed * moveSpeedMod * difficulty);

			modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (direction[CUR_DIR].x < 0.0f)
		{
			modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x + 1.0f, position.y + 1.0f, 0.0f) + glm::vec3(direction[CUR_DIR].x, direction[CUR_DIR].y, 0.0f) * acc * moveSpeed * moveSpeedMod * difficulty);

			modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}

	}
	else
	{
		/* change direction so that pacman doesn't stop moving */
		if (direction[CUR_DIR].x)
		{
			size_t alt_x = (size_t)(x - direction[CUR_DIR].x);
			size_t alt_y0 = y - 1, alt_y1 = y + 1 % GRID_HEIGHT;

			if (!(grid[alt_x + alt_y0 * GRID_WIDTH] & eCellType::WALL))
				direction[CUR_DIR] = { 0.0f, -1.0f };
			else if (!(grid[alt_x + alt_y1 * GRID_WIDTH] & eCellType::WALL))
				direction[CUR_DIR] = { 0.0f, 1.0f };
			else
				direction[CUR_DIR] = { 0.0f, 0.0f };
		}
		else if (direction[CUR_DIR].y)
		{
			size_t alt_y = (size_t)(y - direction[CUR_DIR].y);
			size_t alt_x0 = x - 1, alt_x1 = x + 1 % GRID_WIDTH;

			if (!(grid[alt_x0 + alt_y * GRID_WIDTH] & eCellType::WALL))
				direction[CUR_DIR] = { -1.0f, 0.0f };
			else if (!(grid[alt_x1 + alt_y * GRID_WIDTH] & eCellType::WALL))
				direction[CUR_DIR] = { 1.0f, 0.0f };
			else
				direction[CUR_DIR] = { 0.0f, 0.0f };
		}
		
	}
	/* cells contains fruit/powerup */
	if (grid[(int32_t)position.x + (int32_t)position.y * GRID_WIDTH] & eCellType::FRUIT0)
	{
		points = 10;
		moveSpeedMod = 1.0f;
		grid[x + y * GRID_WIDTH] = eCellType::EMPTY;
		shouldUpdate = true;
		AudioManager::playSound(PACMAN_EAT_FRUIT);
	}

	if (grid[(int32_t)position.x + (int32_t)position.y * GRID_WIDTH] & eCellType::FRUIT1)
	{
		points = 50;
		moveSpeedMod = 1.0f;
		grid[x + y * GRID_WIDTH] = eCellType::EMPTY;
		poweredUp = true;
		shouldUpdate = true;
		AudioManager::playSound(PACMAN_EAT_POWERUP);
	}	
	return points;
}