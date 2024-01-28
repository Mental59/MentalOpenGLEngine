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

void ShaderProgram::SetUniform1i(const std::string& name, GLint value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniform1f(const std::string& name, GLfloat value)
{
	glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniformVec4(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void ShaderProgram::SetUniformMat4(const std::string& name, const GLfloat* data)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, data);
}

void ShaderProgram::SetUniformVec3(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
	glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

GLint ShaderProgram::GetUniformLocation(const std::string& name)
{
	if (mUniformLocationCache.find(name) != mUniformLocationCache.end())
		return mUniformLocationCache[name];

	GLint location = glGetUniformLocation(mID, name.c_str());
	if (location == -1)
	{
		std::cout << "Warning uniform '" << name << "' doesn't exist" << std::endl;
	}

	mUniformLocationCache[name] = location;
	return location;
}
