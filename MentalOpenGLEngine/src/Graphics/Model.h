#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include "Graphics/Mesh.h"
#include "Graphics/ShaderProgram.h"

class Model
{
public:
	virtual ~Model();
	void Load(const std::string& path);
	void Draw(ShaderProgram& shader);
	void Draw(ShaderProgram& shader, const Core::Transform& transform);

	inline bool HasTextures() const { return mLoadedTextures.size() > 0; }
	inline const Core::Transform& GetTransform() const { return mTransform; }
	void SetDefaultTexture(const Core::Texture& texture);
	void SetTransform(const Core::Transform& transform);
	bool HasDefaultTexture(Core::TextureType textureType) const;

private:
	void ProcessNode(struct aiNode* node, const struct aiScene* scene);
	void ProcessMesh(struct aiMesh* mesh, const struct aiScene* scene, std::shared_ptr<Mesh> resMesh);
	std::vector<Core::Texture> LoadMaterialTextures(
		struct aiMaterial* material,
		enum aiTextureType textureType,
		Core::TextureType coreTextureType
	);
	void AddDefaultTexture(std::vector<Core::Texture>* textures, Core::TextureType textureType);

	Core::Transform mTransform;
	std::vector <std::shared_ptr<Mesh>> mMeshes;
	std::string mDirectory;
	std::unordered_map<std::string, unsigned int> mLoadedTextures;
	std::unordered_map<Core::TextureType, Core::Texture> mDefaultTextures;
};
