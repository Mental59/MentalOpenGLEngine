#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader(const char* sourcePath, GLenum type) :
	mType(type)
{
	mSource = ReadSource(sourcePath);
}

Shader::~Shader()
{
	glDeleteShader(mID);
}

void Shader::Compile()
{
	mID = Compile(mSource.c_str(), mType);
}

std::string Shader::ReadSource(const char* sourcePath)
{
	std::ifstream stream(sourcePath);
	std::stringstream sourceStream;
	sourceStream << stream.rdbuf();
	return sourceStream.str();
}

GLuint Shader::Compile(const char* source, GLenum type)
{
	GLuint id = glCreateShader(type);

	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	GLint compileStatus;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);

	if (compileStatus == GL_FALSE)
	{
		GLint infoLength;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLength);

		if (infoLength > 0)
		{
			char* message = (char*)alloca(infoLength * sizeof(char));
			glGetShaderInfoLog(id, infoLength, nullptr, message);
			std::cout << "Failed to compile shader" << std::endl;
			std::cout << message << std::endl;
		}

		glDeleteShader(id);
		return 0;
	}

	return id;
}
