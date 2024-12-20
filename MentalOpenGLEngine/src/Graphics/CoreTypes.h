#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <vector>

#ifndef DIFFUSE_TEXTURE_NAME
#define DIFFUSE_TEXTURE_NAME "uMaterial.diffuseTexture{}"
#endif // !DIFFUSE_TEXTURE_NAME

#ifndef SPECULAR_TEXTURE_NAME
#define SPECULAR_TEXTURE_NAME "uMaterial.specularTexture{}"
#endif // !SPECULAR_TEXTURE_NAME

#ifndef NORMAL_TEXTURE_NAME
#define NORMAL_TEXTURE_NAME "uMaterial.normalTexture{}"
#endif // !NORMAL_TEXTURE_NAME

#ifndef HEIGHT_TEXTURE_NAME
#define HEIGHT_TEXTURE_NAME "uMaterial.heightTexture{}"
#endif // !HEIGHT_TEXTURE_NAME


namespace Core
{
	enum TextureType
	{
		Diffuse = 0, Specular, Normal, Height, TextureTypeCount
	};

	struct BuildTextureOptions
	{
		const char* path;
		const char* uniformName;
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TextureCoordinates;
		glm::vec3 Tangent;
	};

	struct Texture
	{
		unsigned int ID;
		TextureType Type;
	};

	struct Transform
	{
		glm::vec3 Position{ 0.0f };
		glm::vec3 Scale{ 1.0f };

		float RotationAngle{ 0.0f };
		glm::vec3 RotationAxis{ 1.0f, 0.0f, 0.0f };
	};

	struct TextureImport
	{
		const char* path;
		TextureType type;
	};

	struct ModelImport
	{
		const char* path;
		Transform transform;
		bool flipTexturesVertically = false;
		std::vector<TextureImport> textureImports;
	};
}
