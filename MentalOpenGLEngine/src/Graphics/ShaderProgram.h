#pragma once

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

private:
	GLuint mID;
};