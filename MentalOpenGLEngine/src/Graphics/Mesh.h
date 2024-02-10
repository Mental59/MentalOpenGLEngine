#pragma once

#include <vector>
#include "CoreTypes.h"
#include "Graphics/ShaderProgram.h"

class Mesh
{
public:
	Mesh(const std::vector<Core::Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Core::Texture>& textures);
	virtual ~Mesh();

	void Draw(ShaderProgram& shader);

private:
	void SetupMesh(const std::vector<Core::Vertex>& vertices, const std::vector<unsigned int>& indices);

	unsigned int mVAO, mVBO, mEBO;
	size_t mIndicesSize;

	std::vector<Core::Texture> mTextures;
};

