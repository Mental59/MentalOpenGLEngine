#pragma once

#include <vector>
#include "CoreTypes.h"
#include "Graphics/ShaderProgram.h"

class Mesh
{
public:
	Mesh();
	virtual ~Mesh();

	void Draw(ShaderProgram& shader);
	void Setup(const std::vector<Core::Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Core::Texture>& textures);

private:
	unsigned int mVAO, mVBO, mEBO;
	unsigned int mNumIndices, mNumVertices;

	std::vector<Core::Texture> mTextures;
};

