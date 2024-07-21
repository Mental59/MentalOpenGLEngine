#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <vector>

#ifndef ALBEDO_TEXTURE_NAME
#define ALBEDO_TEXTURE_NAME "uMaterial.albedoTexture{}"
#endif // !ALBEDO_TEXTURE_NAME

#ifndef METALLIC_TEXTURE_NAME
#define METALLIC_TEXTURE_NAME "uMaterial.metallicTexture{}"
#endif // !METALLIC_TEXTURE_NAME

#ifndef ROUGHNESS_TEXTURE_NAME
#define ROUGHNESS_TEXTURE_NAME "uMaterial.roughnessTexture{}"
#endif // !ROUGHNESS_TEXTURE_NAME

#ifndef AMBIENT_OCCLUSION_TEXTURE_NAME
#define AMBIENT_OCCLUSION_TEXTURE_NAME "uMaterial.ambientOcclusionTexture{}"
#endif // !AMBIENT_OCCLUSION_TEXTURE_NAME

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
		Albedo = 0,
		Metallic,
		Roughness,
		AmbientOcclusion,
		Normal,
		Height,
		TextureTypeCount
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
