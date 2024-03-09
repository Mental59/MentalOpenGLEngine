#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/matrix.hpp>
#include "Graphics/Mesh.h"
#include "Graphics/ShaderProgram.h"

class Model
{
public:
	virtual ~Model();
	Model(bool flipTexturesVertically = false);
	void Load(const std::string& path);
	void Draw(ShaderProgram& shader);
	void Draw(ShaderProgram& shader, const Core::Transform& transform);
	void Draw(ShaderProgram& shader, const glm::mat4& modelMat);
	void DrawInstanced(ShaderProgram& shader, int n);

	inline bool HasTextures() const { return mLoadedTextures.size() > 0; }
	inline const Core::Transform& GetTransform() const { return mTransform; }
	void SetDefaultTexture(const Core::Texture& texture);
	void SetTransform(const Core::Transform& transform);
	bool HasDefaultTexture(Core::TextureType textureType) const;

	void SetupInstancedDrawing(glm::mat4* instanceMatrices, size_t size, unsigned int location);

private:
	void ProcessNode(struct aiNode* node, const struct aiScene* scene);
	void ProcessMesh(struct aiMesh* mesh, const struct aiScene* scene, std::shared_ptr<Mesh> resMesh);
	std::vector<Core::Texture> LoadMaterialTextures(
		struct aiMaterial* material,
		enum aiTextureType textureType,
		Core::TextureType coreTextureType
	);
	void AddDefaultTexture(std::vector<Core::Texture>* textures, Core::TextureType textureType);

	unsigned int mInstanceMatrixVBO;
	bool mFlipTexturesVertically;
	Core::Transform mTransform;
	std::vector <std::shared_ptr<Mesh>> mMeshes;
	std::string mDirectory;
	std::unordered_map<std::string, unsigned int> mLoadedTextures;
	std::unordered_map<Core::TextureType, Core::Texture> mDefaultTextures;
};
