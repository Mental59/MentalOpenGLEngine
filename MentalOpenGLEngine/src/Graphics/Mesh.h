#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "CoreTypes.h"
#include "Graphics/ShaderProgram.h"

class Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	void Draw(ShaderProgram& shader);
	void DrawInstanced(ShaderProgram& shader, int n);
	void Setup(const std::vector<Core::Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Core::Texture>& textures);

	inline unsigned int GetVAO() const { return mVAO; }

private:
	void BindTextures(ShaderProgram& shader);
	void UnbindTextures();
	void SetTexture(
		ShaderProgram& shader,
		const std::string& textureName,
		unsigned int unit,
		unsigned int textureId
	) const;

	unsigned int mVAO, mVBO, mEBO;
	unsigned int mNumIndices, mNumVertices;

	std::unordered_map<Core::TextureType, Core::Texture> mTextures;
};

