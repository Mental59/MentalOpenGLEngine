#pragma once

#include <glad/glad.h>
#include <string>

class Shader
{
public:
	enum ShaderType
	{
		Vertex, Fragment, Geometry
	};

	Shader(const char* sourcePath, ShaderType type);
	virtual ~Shader();

	inline const char* GetSource() const { return mSource.c_str(); }
	inline GLuint GetID() const { return mID; }
	inline GLenum GetType() const { return mType; }

	void Compile();

private:
	std::string ReadSource(const char* sourcePath);
	GLuint Compile(const char* source, GLenum type);

	std::string mSource;
	GLenum mType;
	GLuint mID;
};
