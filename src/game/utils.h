#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <GL\glew.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>

/*/// macros ///*/

#ifndef NDEBUG
#define ASSERT(x, msg) if (!x) printf("%s\nLine: %d\nFunction: %s\n", msg, __LINE__, __FUNCTION__)
#define ERR(x, msg) if (!x) printf("[ERROR]: %s\n", msg)
#define INFO(msg) printf("[INFO]: %s\n", msg)
#define GL_LOG(msg) printf("[OPENGL]: %s\n", msg)
#define GAME_LOG(msg) printf("[GAME]: %s\n", msg)
#define LOG_FPS(delta)	std::string fpsString("[FPS]: ");\
						fpsString += std::to_string(1.0 / delta);\
						fpsString += "\t[FRAMETIME]: ";\
						fpsString += std::to_string(delta * 1e3);\
						fpsString += "ms";\
						INFO(fpsString.c_str())
#else
#define ASSERT(x, msg) x
#define ERR(x, msg)
#define INFO(msg)
#define GL_LOG(msg)
#define GAME_LOG(msg)
#define LOG_FPS(delta)
#endif

constexpr int GRID_WIDTH = 28;
constexpr int GRID_HEIGHT = 31;
constexpr float TEXTURE_DIVS_U = 1.0f;
constexpr float TEXTURE_DIVS_V = 1.0f;

/*/// enums ///*/

enum eCellType {
	EMPTY = 0,
	WALL = 1,
	WALL_TOP_LEFT = ((1 << 1) | 1),
	WALL_TOP_RIGHT = ((1 << 2) | 1),
	WALL_BOTTOM_LEFT = ((1 << 3) | 1),
	WALL_BOTTOM_RIGHT = ((1 << 4) | 1),
	FRUIT0 = (1 << 5),
	FRUIT1 = (1 << 6),
	FRUIT2 = (1 << 7),
	TELEPORT = ((1 << 8)),
	WALL_VERTICAL = ((1 << 9) | 1),
	WALL_HORIZONTAL = ((1 << 10) | 1),
	INACCESSIBLE = ((1 << 11) | 1)
};

static const char* Sound[] = {
	"GAME_START",
	"GAME_END",
	"PACMAN_EAT_POWERUP",
	"PACMAN_EAT_FRUIT",
	"PACMAN_EAT_GHOST"
};

enum eSound {
	GAME_START = 0,
	GAME_END,
	PACMAN_EAT_POWERUP,
	PACMAN_EAT_FRUIT,
	PACMAN_EAT_GHOST
};

/*/// structs to help with opengl ///*/

struct Vertex
{
	glm::vec2 position;
	glm::vec2 texCoords;
};

struct Quad
{
	Quad() {}
	Quad(const glm::vec2& position, const glm::vec2& size)
	{
		vertices[0] = { position, {0.0f, 0.0f} };
		vertices[1] = { {position.x + size.x, position.y}, {1.0f, 0.0f} };
		vertices[2] = { {position.x + size.x, position.y + size.y}, {1.0f, 1.0f} };
		vertices[3] = { {position.x, position.y + size.y}, {0.0f, 1.0f} };
	}
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

/*/// utility functions ///*/

template<typename T>
inline T sgn(T val)
{
	return T((T(0) < val) - (val < T(0)));
}

void readShaders(const char* filepath, std::string & vertexShader, std::string & fragmentShader);
uint32_t createShader(GLenum shaderType, const char* shaderSource);
void dumpData(const void* data, int32_t size, const std::string& fileName);
glm::vec2 signum(const glm::vec2& vector);
void showConsoleWindow();
void hideConsoleWindow();