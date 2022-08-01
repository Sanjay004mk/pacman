#include "game_manager.h"

#include "data.h"

/////////////////////////////////////
/// class: GameManager //////////////
/////////////////////////////////////

GameManager::GameManager(float aspectRatio)
{
	init();
	loadmap();

	animFrame = new int32_t[100];
	for (size_t i = 0; i < 100; i++)
	{
		animFrame[i] = rand() % 2;
	}

	/* set projection matrix so that the grid is always centered on the screen */
	glUseProgram(cellShaderProgram);
	projMatrix = aspectRatio >= (float)(GRID_WIDTH) / (float)(GRID_HEIGHT + 3) ? glm::ortho(-((float)(GRID_HEIGHT + 3) * aspectRatio - (float)GRID_WIDTH) / 2.0f, (float)GRID_WIDTH + ((float)(GRID_HEIGHT + 3) * aspectRatio - (float)GRID_WIDTH) / 2.0f, -1.0f, (float)GRID_HEIGHT + 2, -2.0f, 2.0f) : glm::ortho(0.0f, (float)GRID_WIDTH, (- ((float)GRID_WIDTH * (1.0f / aspectRatio) - (float)(GRID_HEIGHT + 2)) / 2.0f) - 1.0f, (float)(GRID_HEIGHT + 2) + ((float)GRID_WIDTH * (1.0f / aspectRatio) - (float)(GRID_HEIGHT + 2)) / 2.0f, -2.0f, 2.0f);
	glUniformMatrix4fv(glGetUniformLocation(cellShaderProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
	glUniform1iv(glGetUniformLocation(cellShaderProgram, "animFrame"), 100, animFrame);
	glUseProgram(uiProgram);	
	glUniformMatrix4fv(glGetUniformLocation(uiProgram, "projection"), 1, GL_FALSE, &projMatrix[0][0]);
	glUseProgram(0);	

	genBuffers();
	loadWallTexture();
	loadPacmanTexture();
	loadNumTexture();

	pacman = new Pacman(projMatrix, pacmanTexture);
	ghostManager = new GhostManager(projMatrix, cellTypes);

	AudioManager::playSound(GAME_START);
}

GameManager::~GameManager()
{
	deleteBuffers();

	glDeleteBuffers(1, &lvbo);
	glDeleteBuffers(1, &libo);
	glDeleteVertexArrays(1, &lvao);
	glDeleteBuffers(1, &pvbo);
	glDeleteBuffers(1, &pibo);
	glDeleteVertexArrays(1, &pvao);

	glDeleteTextures(1, &wallTexture);
	glDeleteTextures(1, &pacmanTexture);
	glDeleteTextures(1, &numTexture);
	glDeleteProgram(cellShaderProgram);
	glDeleteProgram(uiProgram);
	
	delete[] animFrame;
	delete[] cellTypes;
	delete pacman;
	delete ghostManager;
}

void GameManager::init()
{
	/* opengl stuff */
	cellShaderProgram = glCreateProgram();
	std::string vShader = sShaders[CELL_SHADER].vertex, fShader = sShaders[CELL_SHADER].fragment;
	const char* vSource = vShader.c_str(), * fSource = fShader.c_str();

	uint32_t vertexShader = createShader(GL_VERTEX_SHADER, vSource);
	glAttachShader(cellShaderProgram, vertexShader);
	glDeleteShader(vertexShader);

	uint32_t fragmentShader = createShader(GL_FRAGMENT_SHADER, fSource);
	glAttachShader(cellShaderProgram, fragmentShader);
	glDeleteShader(fragmentShader);

	glLinkProgram(cellShaderProgram);
	int32_t success;
	glGetProgramiv(cellShaderProgram, GL_LINK_STATUS, &success);
	if (success != GL_TRUE)
	{
		int32_t len;
		glGetProgramiv(cellShaderProgram, GL_INFO_LOG_LENGTH, &len);
		char* msg = new char[len];
		glGetProgramInfoLog(cellShaderProgram, len, &len, msg);
		GL_LOG(msg);
		delete[] msg;
	}
	initUI();
}

void GameManager::initUI()
{
	uiProgram = glCreateProgram();
	std::string vShader = sShaders[UI_SHADER].vertex, fShader = sShaders[UI_SHADER].fragment;
	const char* vSource = vShader.c_str(), * fSource = fShader.c_str();

	uint32_t vertexShader = createShader(GL_VERTEX_SHADER, vSource);
	glAttachShader(uiProgram, vertexShader);
	glDeleteShader(vertexShader);

	uint32_t fragmentShader = createShader(GL_FRAGMENT_SHADER, fSource);
	glAttachShader(uiProgram, fragmentShader);
	glDeleteShader(fragmentShader);

	glLinkProgram(uiProgram);
	int32_t success;
	glGetProgramiv(uiProgram, GL_LINK_STATUS, &success);
	if (success != GL_TRUE)
	{
		int32_t len;
		glGetProgramiv(uiProgram, GL_INFO_LOG_LENGTH, &len);
		char* msg = new char[len];
		glGetProgramInfoLog(uiProgram, len, &len, msg);
		GL_LOG(msg);
		delete[] msg;
	}

	int32_t p = points;
	for (int32_t i = (int32_t)(pointsNum.size() - 1); i >= 0; i--)
	{
		int32_t n = p % 10;
		p = p / 10;
		float u = 0.0f + (0.1f * (float)n), v = 0.0f;
		pointsNum[i].vertices[0].position = glm::vec2(1.0f + (float)i, (float)GRID_HEIGHT);
		pointsNum[i].vertices[1].position = glm::vec2(2.0f + (float)i, (float)GRID_HEIGHT);
		pointsNum[i].vertices[2].position = glm::vec2(2.0f + (float)i, (float)GRID_HEIGHT + 1.0f);
		pointsNum[i].vertices[3].position = glm::vec2(1.0f + (float)i, (float)GRID_HEIGHT + 1.0f);
		
		pointsNum[i].vertices[0].texCoords = glm::vec2(u + (0.125f / 32.0f), v + 1.0f);
		pointsNum[i].vertices[1].texCoords = glm::vec2(u + 0.1f - (0.125f / 32.0f),	v + 1.0f);
		pointsNum[i].vertices[2].texCoords = glm::vec2(u + 0.1f - (0.125f / 32.0f),	v);
		pointsNum[i].vertices[3].texCoords = glm::vec2(u + (0.125f / 32.0f), v);

		pointsNumIndices[i].setQuadIndices((uint32_t)(4 - i) * 4);
	}

	glGenVertexArrays(1, &pvao);
	glBindVertexArray(pvao);
	glGenBuffers(1, &pvbo);
	glBindBuffer(GL_ARRAY_BUFFER, pvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointsNum), pointsNum.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(sizeof(float) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &pibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointsNumIndices), pointsNumIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	for (size_t i = 0; i < pacmanLives; i++)
	{
		lives[i].vertices[0].position = glm::vec2(1.0f + (float)i + (i * 0.2f), -1.0f);
		lives[i].vertices[1].position = glm::vec2(2.0f + (float)i + (i * 0.2f), -1.0f);
		lives[i].vertices[2].position = glm::vec2(2.0f + (float)i + (i * 0.2f), 0.0f);
		lives[i].vertices[3].position = glm::vec2(1.0f + (float)i + (i * 0.2f), 0.0f);

		lives[i].vertices[0].texCoords = glm::vec2(0.0f + (0.125f / 32.0f), 0.0f);
		lives[i].vertices[1].texCoords = glm::vec2(0.125f - (0.125f / 32.0f), 0.0f);
		lives[i].vertices[2].texCoords = glm::vec2(0.125f - (0.125f / 32.0f), 1.0f);
		lives[i].vertices[3].texCoords = glm::vec2(0.0f + (0.125f / 32.0f), 1.0f);

		livesIndices[i].setQuadIndices((uint32_t)(i * 4));
	}

	glGenVertexArrays(1, &lvao);
	glBindVertexArray(lvao);
	glGenBuffers(1, &lvbo);
	glBindBuffer(GL_ARRAY_BUFFER, lvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lives), lives.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(sizeof(float) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &libo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, libo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(livesIndices), livesIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GameManager::updateUI()
{
	int32_t p = points;
	for (int32_t i = (int32_t)(pointsNum.size() - 1); i >= 0; i--)
	{
		int32_t n = p % 10;
		p = p / 10;
		float u = 0.0f + (0.1f * (float)n), v = 0.0f;
		pointsNum[i].vertices[0].position = glm::vec2(1.0f + (float)i, (float)GRID_HEIGHT);
		pointsNum[i].vertices[1].position = glm::vec2(2.0f + (float)i, (float)GRID_HEIGHT);
		pointsNum[i].vertices[2].position = glm::vec2(2.0f + (float)i, (float)GRID_HEIGHT + 1.0f);
		pointsNum[i].vertices[3].position = glm::vec2(1.0f + (float)i, (float)GRID_HEIGHT + 1.0f);

		pointsNum[i].vertices[0].texCoords = glm::vec2(u + (0.125f / 32.0f), v + 1.0f);
		pointsNum[i].vertices[1].texCoords = glm::vec2(u + 0.1f - (0.125f / 32.0f), v + 1.0f);
		pointsNum[i].vertices[2].texCoords = glm::vec2(u + 0.1f - (0.125f / 32.0f), v);
		pointsNum[i].vertices[3].texCoords = glm::vec2(u + (0.125f / 32.0f), v);

	}

	glBindVertexArray(pvao);
	glBindBuffer(GL_ARRAY_BUFFER, pvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointsNum), pointsNum.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void GameManager::deleteBuffers()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
}

void GameManager::updateBuffers()
{
	deleteBuffers();
	genBuffers();
}

void GameManager::genBuffers()
{
	/* set all the vertex data for the grid cells */
	uint32_t index = 0;
	for (int32_t y = 0; y < GRID_HEIGHT; y++)
	{
		for (int32_t x = 0; x < GRID_WIDTH; x++)
		{
			cells[x][y].vertices[0].position = glm::vec2(x,			y);
			cells[x][y].vertices[1].position = glm::vec2(x + 1.0f,	y);
			cells[x][y].vertices[2].position = glm::vec2(x + 1.0f,	y + 1.0f);
			cells[x][y].vertices[3].position = glm::vec2(x,			y + 1.0f);
			if (cellTypes[x + y * GRID_WIDTH] == eCellType::WALL_HORIZONTAL)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 0.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 1.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 1.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 0.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::WALL_VERTICAL)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 2.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 3.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 3.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 2.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::WALL_BOTTOM_LEFT)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 4.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 5.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 5.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 4.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::WALL_TOP_LEFT)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 6.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 7.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 7.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 6.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::WALL_BOTTOM_RIGHT)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 8.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 9.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 9.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 8.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::WALL_TOP_RIGHT)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 10.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 11.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 11.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 10.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::FRUIT0)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 12.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 13.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 13.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 12.0f + (0.125f / 32.0f), 1.0f);
			}
			else if (cellTypes[x + y * GRID_WIDTH] == eCellType::FRUIT1)
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0625f * 14.0f + (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0625f * 15.0f - (0.125f / 32.0f), 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0625f * 15.0f - (0.125f / 32.0f), 1.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0625f * 14.0f + (0.125f / 32.0f), 1.0f);
			}
			else
			{
				cells[x][y].vertices[0].texCoords = glm::vec2(0.0f, 0.0f);
				cells[x][y].vertices[1].texCoords = glm::vec2(0.0f, 0.0f);
				cells[x][y].vertices[2].texCoords = glm::vec2(0.0f, 0.0f);
				cells[x][y].vertices[3].texCoords = glm::vec2(0.0f, 0.0f);
			}


			cellsIndices[x][y].setQuadIndices(index);
			index += 4;

		}
	}

	/* opengl stuff */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cells), cells.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(sizeof(float) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cellsIndices), cellsIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GameManager::loadWallTexture()
{
	int32_t width = sTextures[WALL_TEXTURE].width, height = sTextures[WALL_TEXTURE].height, channels = sTextures[WALL_TEXTURE].channels;
	const uint8_t* data = sTextures[WALL_TEXTURE].data.data();

	glGenTextures(1, &wallTexture);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameManager::loadPacmanTexture()
{
	int32_t width = sTextures[PACMAN_TEXTURE].width, height = sTextures[PACMAN_TEXTURE].height, channels = sTextures[PACMAN_TEXTURE].channels;
	const uint8_t* data = sTextures[PACMAN_TEXTURE].data.data();

	glGenTextures(1, &pacmanTexture);
	glBindTexture(GL_TEXTURE_2D, pacmanTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameManager::loadNumTexture()
{
	int32_t width = sTextures[NUM_TEXTURE].width, height = sTextures[NUM_TEXTURE].height, channels = sTextures[NUM_TEXTURE].channels;
	const uint8_t* data = sTextures[NUM_TEXTURE].data.data();

	glGenTextures(1, &numTexture);
	glBindTexture(GL_TEXTURE_2D, numTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameManager::resetLevel()
{
	resetPacmanAndGhosts();

	if (--pacmanLives <= 0)
		gameReset();
	else
		AudioManager::playSound(GAME_START);
}

void GameManager::resetPacmanAndGhosts()
{
	delete pacman;
	pacman = new Pacman(projMatrix, pacmanTexture);

	delete ghostManager;
	ghostManager = new GhostManager(projMatrix, cellTypes);

	gameStart = true;
}

void GameManager::gameReset()
{
	loadmap();
	pacmanLives = 3;
	points = 0;
	updateBuffers();
	AudioManager::playSound(GAME_START);
}

int32_t GameManager::getFruitCount()
{
	int32_t count = 0;
	for (size_t i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
	{
		if (cellTypes[i] & eCellType::FRUIT0)
			count++;
	}
	return count;
}

void GameManager::loadmap()
{
	if (cellTypes)
		delete[] cellTypes;

	cellTypes = new eCellType[GRID_HEIGHT * GRID_WIDTH];
	
	for (size_t i = 0; i < GRID_HEIGHT * GRID_WIDTH; i++)
		cellTypes[i] = sGridCells[i];
}

void GameManager::render(float delta)
{
	glUseProgram(cellShaderProgram);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	/* grid cells */
	glDrawElements(GL_TRIANGLES, 6 * GRID_HEIGHT * GRID_WIDTH, GL_UNSIGNED_INT, nullptr);

	/* ui elements */
	glUseProgram(uiProgram);
	glBindTexture(GL_TEXTURE_2D, pacmanTexture);
	glBindVertexArray(lvao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, libo);
	glDrawElements(GL_TRIANGLES, 6 * pacmanLives, GL_UNSIGNED_INT, nullptr);
	glBindTexture(GL_TEXTURE_2D, numTexture);
	glBindVertexArray(pvao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pibo);
	glDrawElements(GL_TRIANGLES, 6 * (int32_t)pointsNum.size(), GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	pacman->render(delta);
	ghostManager->render();
}

void GameManager::update(float delta)
{
	/* pause for 2 seconds at the start of level */
	static float waitTime = 0.0f;
	if (gameStart)
	{
		if ((waitTime += delta) > 2.0f)
		{
			waitTime = 0.0f;
			gameStart = false;
		}
		return;
	}

	/* randomize texture of tiles between lighter and darker texture*/
	static float animTimer = 0.0f;
	if ((animTimer += delta) > (8.0f))
	{
		animTimer = 0.0f;
		for (size_t i = 0; i < 100; i++)
		{
			if (rand() % 2)
				animFrame[i] = rand() % 2;
		}
		glUseProgram(cellShaderProgram);
		glUniform1iv(glGetUniformLocation(cellShaderProgram, "animFrame"), 100, animFrame);
		glUseProgram(0);
	}

	bool shouldUpdate = false;
	if ((points += pacman->move(delta, cellTypes, shouldUpdate)) >= 2400)
		if (getFruitCount() <= 0)
			gameWon();

	if (shouldUpdate)
		updateBuffers();

	bool ateGhost = false;
	if (ghostManager->ghostCaughtPacman(delta, cellTypes, ateGhost, pacman->position, pacman->direction[CUR_DIR], pacman->poweredUp))
		gameOver();

	if (ateGhost)
		pacman->poweredUp = false;

	updateUI();
}

void GameManager::gameWon()
{
	static float difficulty = 1.0f;
	difficulty = std::min(2.0f, difficulty += 0.1f);

	loadmap();
	resetPacmanAndGhosts();
	pacmanLives = 3;
	ghostManager->setDifficulty(difficulty);
	pacman->difficulty = difficulty;
	updateBuffers();
	AudioManager::playSound(GAME_START);
}

void GameManager::gameOver()
{
	AudioManager::playSound(GAME_END);
	AudioManager::waitForEnd(GAME_END);
	resetLevel();
}

void GameManager::setMatrix(float aspectRatio)
{
	/* set projection matrix so that the grid is always centered on the screen */
	if (aspectRatio >= (float)(GRID_WIDTH) / (float)(GRID_HEIGHT + 3))
		projMatrix = glm::ortho(-((float)(GRID_HEIGHT + 3) * aspectRatio - (float)GRID_WIDTH) / 2.0f, (float)GRID_WIDTH + ((float)(GRID_HEIGHT + 3) * aspectRatio - (float)GRID_WIDTH) / 2.0f, -1.0f, (float)GRID_HEIGHT + 2, -2.0f, 2.0f);
	else
		projMatrix = glm::ortho(0.0f, (float)GRID_WIDTH, (- ((float)GRID_WIDTH * (1.0f / aspectRatio) - (float)(GRID_HEIGHT + 2)) / 2.0f) - 1.0f, (float)(GRID_HEIGHT + 2) + ((float)GRID_WIDTH * (1.0f / aspectRatio) - (float)(GRID_HEIGHT + 2)) / 2.0f, -2.0f, 2.0f);
	glUseProgram(cellShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(cellShaderProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
	glUseProgram(uiProgram);
	glUniformMatrix4fv(glGetUniformLocation(uiProgram, "projection"), 1, GL_FALSE, &projMatrix[0][0]);
	glUseProgram(0);
	pacman->updateMatrix(projMatrix);
	ghostManager->updateMatrix(projMatrix);
}