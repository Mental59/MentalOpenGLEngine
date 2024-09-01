#pragma once

#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include <string>
#include "Text/TextCharacter.h"

class TextRenderer
{
public:
	~TextRenderer();

	bool Build(const int width, const int height);
	void Draw(const std::string& text, glm::vec2 pos, glm::vec3 color, float scale);

private:
	int mWidth, mHeight;
	glm::mat4 mProjection;

	unsigned int mVAO, mVBO;
	ShaderProgram mShader;

	std::unordered_map<unsigned char, TextCharacter> mTextCharacters;
};
