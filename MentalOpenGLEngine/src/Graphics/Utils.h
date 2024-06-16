#pragma once

#include <iostream>
#include <glad/glad.h>
#include "External/stb_image.h"

struct ImageFormat
{
	GLenum DataFormat = 0;
	GLint InternalFormat = 0;
};

inline ImageFormat GetImageFormat(int numChannels, bool srgb)
{
	GLenum dataFormat = 0;
	GLint internalFormat = 0;

	if (numChannels == 1)
	{
		dataFormat = GL_RED;
		internalFormat = GL_R8;
	}
	else if (numChannels == 3)
	{
		dataFormat = GL_RGB;
		internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
	}
	else if (numChannels == 4)
	{
		dataFormat = GL_RGBA;
		internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	}

	return { dataFormat, internalFormat };
}

inline unsigned int GLLoadTextureFromFile(const char* texturePath, bool flipVertically = false, bool srgb = false)
{
	stbi_set_flip_vertically_on_load(flipVertically);

	GLuint textureID = 0;

	int width, height, numChannels;
	unsigned char* data = stbi_load(texturePath, &width, &height, &numChannels, 0);

	if (data)
	{
		ImageFormat imageFormat = GetImageFormat(numChannels, srgb);

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, imageFormat.InternalFormat, width, height, 0, imageFormat.DataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		const char* reason = stbi_failure_reason();
		std::cout << "Failed to load texture at path " << texturePath << "\nReason: " << reason << std::endl;
	}

	stbi_image_free(data);

	return textureID;
}

inline unsigned int GLLoadCubemap(const char* faces[6], bool srgb = false)
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
			ImageFormat imageFormat = GetImageFormat(numChannels, srgb);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, imageFormat.InternalFormat, width, height, 0, imageFormat.DataFormat, GL_UNSIGNED_BYTE, data);
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
