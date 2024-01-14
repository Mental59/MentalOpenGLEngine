#pragma once

#include <glad/glad.h>
#include <string>

class Shader
{
public:
	Shader(const char* sourcePath, GLenum type);
	virtual ~Shader();

	inline const char* GetSource() const { return mSource.c_str(); }
	inline GLuint GetID() const { return mID; }
	inline GLenum GetType() const { return mType; }

private:
	std::string ReadShaderSource(const char* sourcePath);
	GLuint CompileShader(const char* source, GLenum type);

	std::string mSource;
	GLenum mType;
	GLuint mID;
};
