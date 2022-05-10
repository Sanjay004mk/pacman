#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <GL\glew.h>

#ifndef NDEBUG
#define ASSERT(x, msg) if (!x) { printf(msg); printf("\nLine: %d\nFunction: ", __LINE__); printf(__FUNCTION__); }
#else
#define ASSERT(x, msg) x
#endif

void readShaders(const char* filepath, std::string & vertexShader, std::string & fragmentShader);
uint32_t createShader(GLenum shaderType, const char* shaderSource);