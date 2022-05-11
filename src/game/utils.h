#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <GL\glew.h>

#ifndef NDEBUG
#define ASSERT(x, msg) if (!x) printf("%s\nLine: %d\nFunction: %s\n", msg, __LINE__, __FUNCTION__)
#define GL_LOG(x) printf("[OPENGL]: %s\n", x)
#define GAME_LOG(x) printf("[GAME]: %s\n", x)
#else
#define ASSERT(x, msg) x
#define GL_LOG(x)
#define GAME_LOG(x)
#endif

void readShaders(const char* filepath, std::string & vertexShader, std::string & fragmentShader);
uint32_t createShader(GLenum shaderType, const char* shaderSource);