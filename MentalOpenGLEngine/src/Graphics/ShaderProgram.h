#pragma once

#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <vector>

class ShaderProgram
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	void Build(const std::vector<class Shader>& shaders);
	void Bind();
	void Unbind();

	inline GLuint GetID() const { return mID; }

	void SetUniform1i(const std::string& name, GLint value);
	void SetUniform1f(const std::string& name, GLfloat value);
	void SetUniformVec4(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void SetUniformMat4(const std::string& name, const GLfloat* data);
	void SetUniformVec3(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
	void SetUniformVec3(const std::string& name, const GLfloat* data);
	void SetUniformVec2(const std::string& name, const GLfloat* data);

	void SetUniformBlockBinding(const char* uniformBlockName, GLuint binding) const;

private:
	GLint GetUniformLocation(const std::string& name);

	GLuint mID;
	std::unordered_map<std::string, GLint> mUniformLocationCache;
};