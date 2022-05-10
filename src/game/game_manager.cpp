#include "game_manager.h"

GameManager::GameManager()
{
	program = glCreateProgram();
	std::string vShader, fShader;
	readShaders("basic.shader", vShader, fShader);
	const char* vSource = vShader.c_str(), * fSource = fShader.c_str();

	uint32_t vertexShader = createShader(GL_VERTEX_SHADER, vSource);
	glAttachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	
	uint32_t fragmentShader = createShader(GL_FRAGMENT_SHADER, fSource);
	glAttachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);

	glLinkProgram(program);
	int32_t success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success != GL_TRUE)
	{
		int32_t len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		char* msg = new char[len];
		glGetProgramInfoLog(program, len, &len, msg);
		printf(msg);
		delete[] msg;
		__debugbreak();
	}
	glUseProgram(program);


	float tex_u = 1.0f / TEXTURE_DIVS_U;
	float tex_v = 1.0f / TEXTURE_DIVS_V;
	float vertices[] = {
		0.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,		0.0f, tex_v,
		1.0f, 1.0f, 0.0f,		tex_u, tex_v,
		1.0f, 0.0f, 0.0f,		tex_u, 0.0f
	};

	uint8_t indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void*)(sizeof(float) * 3));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GameManager::~GameManager()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
}

void GameManager::render(float delta)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GameManager::update(float delta)
{

}