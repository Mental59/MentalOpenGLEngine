#pragma once

#include <iostream>
#include <glad/glad.h>
#include "External/stb_image.h"

inline unsigned int GLLoadTextureFromFile(const char* texturePath, bool flipVertically = false)
{
	stbi_set_flip_vertically_on_load(flipVertically);

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

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		std::cout << "Failed to load texture at path " << texturePath << std::endl;
	}

	stbi_image_free(data);

	return textureID;
}

inline unsigned int GLLoadCubemap(const char* faces[6])
{
	unsigned int textureId = 0;

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	for (int i = 0; i < 6; i++)
	{
		int width, height, numChannels;
		unsigned char* data = stbi_load(faces[i], &width, &height, &numChannels, 0);

		if (data)
		{
			GLenum format = GL_RGB;
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

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Failed to load cubemap at path " << faces[i] << std::endl;
		}

		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureId;
}
