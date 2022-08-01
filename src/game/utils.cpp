#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Windows.h>
#include "utils.h"

void readShaders(const char* filepath, std::string& vertexShader, std::string& fragmentShader)
{
	std::ifstream file(filepath);
	ERR(!file.fail(), "failed to load shader files!");
	std::stringstream ss[2];
	std::string line;
	size_t index = 0;
	while (std::getline(file, line))
	{
		if (line.find("$TYPE") != std::string::npos)
		{
			if (line.find("VERTEX") != std::string::npos)
				index = 0;
			else
				index = 1;
		}
		else
		{
			ss[index] << line << "\n";
		}
	}
	file.close();
	vertexShader = ss[0].str();
	fragmentShader = ss[1].str();
}

uint32_t createShader(GLenum shaderType, const char* shaderSource)
{
	uint32_t shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, nullptr);
	glCompileShader(shader);
	int32_t success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE)
	{
		int32_t len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		char* msg = new char[len];
		glGetShaderInfoLog(shader, len, &len, msg);
		printf("[OPENGL]: failed to compile");
		shaderType == GL_VERTEX_SHADER ? printf(" vertex ") : printf(" fragment ");
		printf("shader!\n");
		GL_LOG(msg);
		delete[] msg;
	}

	return shader;
}

void dumpData(const void* data, int32_t size, const std::string& fileName)
{
	std::ofstream dump(fileName, std::ios::binary);
	ASSERT(!dump.fail(), "failed to dump data!");
	dump.write((const char*)data, size);
	dump.close();
}

glm::vec2 signum(const glm::vec2& vector)
{
	return { sgn(vector.x), sgn(vector.y) };
}

void showConsoleWindow()
{
	::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}

void hideConsoleWindow()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}