#pragma once

#include <iostream>
#include <glad/glad.h>
#include "External/stb_image.h"

inline unsigned int GLLoadTextureFromFile(const char* texturePath)
{
	stbi_set_flip_vertically_on_load(true);

	GLuint textureID = 0;

	int width, height, numChannels;
	unsigned char* data = stbi_load(texturePath, &width, &height, &numChannels, 0);

	if (data)
	{
		GLenum format = 0;
		if (numChannels == 1)
		{
			format = GL_RED;
		}
		else if (numChannels == 3)
		{
			format = GL_RGB;
		}
		else if (numChannels == 4)
		{
			format = GL_RGBA;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture at path " << texturePath << std::endl;
	}

	stbi_image_free(data);

	return textureID;
}
