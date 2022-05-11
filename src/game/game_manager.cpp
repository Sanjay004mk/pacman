#include "game_manager.h"

GameManager::GameManager(float aspectRatio)
{
	init();

	glUseProgram(cellShaderProgram);
	projMatrix = glm::ortho(-16.0f * aspectRatio, 16.0f * aspectRatio, -16.0f, 16.0f, -2.0f, 2.0f);
	glUniformMatrix4fv(glGetUniformLocation(cellShaderProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
	glUseProgram(0);

	uint32_t index = 0;

	for (int32_t y = 0; y < 32; y++)
	{
		for (int32_t x = 0; x < 32; x++)
		{
			cells[x][y].vertices[0].position = glm::vec2((x - 16), (y - 16));
			cells[x][y].vertices[0].texCoords = glm::vec2(0.0f, 0.0f);

			cells[x][y].vertices[1].position = glm::vec2((x - 16), (y - 16)+ 1.0f);
			cells[x][y].vertices[1].texCoords = glm::vec2(0.0f, 1.0f);

			cells[x][y].vertices[2].position = glm::vec2((x - 16) + 1.0f, (y - 16) + 1.0f);
			cells[x][y].vertices[2].texCoords = glm::vec2(1.0f, 1.0f);

			cells[x][y].vertices[3].position = glm::vec2((x - 16) + 1.0f, (y - 16));
			cells[x][y].vertices[3].texCoords = glm::vec2(1.0f, 0.0f);

			cellsIndices[x][y].setQuadIndices(index);
			index += 4;
		}
	}

	size_t size = sizeof(cells);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cells), cells.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const void*)(sizeof(float) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cellsIndices), cellsIndices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GameManager::~GameManager()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(cellShaderProgram);
}

void GameManager::init()
{
	cellShaderProgram = glCreateProgram();
	std::string vShader, fShader;
	readShaders("cell.shader", vShader, fShader);
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
		printf(msg);
		delete[] msg;
		__debugbreak();
	}
}

void GameManager::render(float delta)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(cellShaderProgram);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, 6 * 32 * 32, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void GameManager::update(float delta)
{

}

void GameManager::setMatrix(float aspectRatio)
{
	projMatrix = aspectRatio >= 1.0f ? glm::ortho(-16.0f * aspectRatio, 16.0f * aspectRatio, -16.0f, 16.0f, -2.0f, 2.0f) : glm::ortho(-16.0f, 16.0f, -16.0f * (1.0f / aspectRatio), 16.0f * (1.0f / aspectRatio), -2.0f, 2.0f);
	glUseProgram(cellShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(cellShaderProgram, "proj"), 1, GL_FALSE, &projMatrix[0][0]);
	glUseProgram(0);
}