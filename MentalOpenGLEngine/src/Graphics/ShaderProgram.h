#pragma once

#include <string>
#include <unordered_map>
#include <glad/glad.h>

class ShaderProgram
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	void Build(
		const char* vertexShaderPath,
		const char* fragmentShaderPath
	);
	void Bind();
	void Unbind();

	inline GLuint GetID() const { return mID; }

	void SetUniform1i(const std::string& name, GLint value);
	void SetUniform1f(const std::string& name, GLfloat value);
	void SetUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void SetUniformMatrix4fv(const std::string& name, const GLfloat* data);

private:
	GLint GetUniformLocation(const std::string& name);

	GLuint mID;
	std::unordered_map<std::string, GLint> mUniformLocationCache;
};