#include "Mesh.h"
#include <iostream>
#include <format>

Mesh::Mesh() : mVAO(0), mVBO(0), mEBO(0), mNumIndices(0), mNumVertices(0)
{

}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mEBO);
}

void Mesh::Draw(ShaderProgram& shader)
{
	unsigned int diffuseTextureNumber = 1;
	unsigned int specularTextureNumber = 1;

	for (int i = 0; i < mTextures.size(); i++)
	{
		if (mTextures[i].Type == Core::Diffuse)
		{
			shader.SetUniform1i(std::format(DIFFUSE_TEXTURE_NAME, diffuseTextureNumber++), i);
		}
		if (mTextures[i].Type == Core::Specular)
		{
			shader.SetUniform1i(std::format(SPECULAR_TEXTURE_NAME, specularTextureNumber++), i);
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, mTextures[i].ID);
	}

	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::Setup(const std::vector<Core::Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Core::Texture>& textures)
{
	mTextures = textures;
	mNumIndices = indices.size();
	mNumVertices = vertices.size();

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mEBO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Core::Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glBindVertexArray(mVAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (const void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (const void*)offsetof(Core::Vertex, Normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (const void*)offsetof(Core::Vertex, TextureCoordinates));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
