#include <iostream>
#include "ShaderProgram.h"
#include "Shader.h"

ShaderProgram::ShaderProgram() : mID(0)
{

}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(mID);
}

void ShaderProgram::Build(
	const char* vertexShaderPath,
	const char* fragmentShaderPath
)
{
	Shader vertexShader(vertexShaderPath, GL_VERTEX_SHADER);
	Shader fragmentShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
	vertexShader.Compile();
	fragmentShader.Compile();

	GLuint shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader.GetID());
	glAttachShader(shaderProgram, fragmentShader.GetID());
	glLinkProgram(shaderProgram);

	GLint programLinkStatus;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programLinkStatus);

	if (programLinkStatus == GL_FALSE)
	{
		GLint infoLen = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 0)
		{
			char* infoLog = (char*)alloca(infoLen);
			glGetProgramInfoLog(shaderProgram, infoLen, NULL, infoLog);
			std::cout << "Shader program linking error" << std::endl << infoLog << std::endl;
		}

		glDeleteProgram(shaderProgram);

		mID = 0;
		return;
	}

	mID = shaderProgram;
}

void ShaderProgram::Bind()
{
	glUseProgram(mID);
}

void ShaderProgram::Unbind()
{
	glUseProgram(0);
}
