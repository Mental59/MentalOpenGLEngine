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
	BindTextures(shader);

	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	UnbindTextures();
}

void Mesh::DrawInstanced(ShaderProgram& shader, int n)
{
	BindTextures(shader);

	glBindVertexArray(mVAO);
	glDrawElementsInstanced(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0, n);
	glBindVertexArray(0);

	UnbindTextures();
}

void Mesh::Setup(const std::vector<Core::Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Core::Texture>& textures)
{
	for (const Core::Texture& texture : textures)
	{
		if (mTextures.find(texture.Type) == mTextures.end())
		{
			mTextures[texture.Type] = texture;
		}
	}

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

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Core::Vertex), (const void*)offsetof(Core::Vertex, Tangent));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::BindTextures(ShaderProgram& shader)
{
	auto diffuseTexture = mTextures.find(Core::Diffuse);
	auto specularTexture = mTextures.find(Core::Specular);
	auto normalTexture = mTextures.find(Core::Normal);
	auto heightTexture = mTextures.find(Core::Height);

	if (diffuseTexture != mTextures.end())
	{
		SetTexture(shader, std::format(DIFFUSE_TEXTURE_NAME, 1), 0, diffuseTexture->second.ID);
	}

	if (specularTexture != mTextures.end())
	{
		SetTexture(shader, std::format(SPECULAR_TEXTURE_NAME, 1), 1, specularTexture->second.ID);
		shader.SetUniform1i("uMaterial.useSpecularTexture", 1);
	}
	else
	{
		shader.SetUniform1i("uMaterial.useSpecularTexture", 0);
	}

	if (normalTexture != mTextures.end())
	{
		SetTexture(shader, std::format(NORMAL_TEXTURE_NAME, 1), 2, normalTexture->second.ID);
		shader.SetUniform1i("uMaterial.useNormalTexture", 1);
	}
	else
	{
		shader.SetUniform1i("uMaterial.useNormalTexture", 0);
	}

	if (heightTexture != mTextures.end())
	{
		SetTexture(shader, std::format(HEIGHT_TEXTURE_NAME, 1), 3, heightTexture->second.ID);
		shader.SetUniform1i("uMaterial.useHeightTexture", 1);
	}
	else
	{
		shader.SetUniform1i("uMaterial.useHeightTexture", 0);
	}
}

void Mesh::UnbindTextures()
{
	for (int i = 0; i < Core::TextureTypeCount; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Mesh::SetTexture(ShaderProgram& shader, const std::string& textureName, unsigned int unit, unsigned int textureId) const
{
	shader.SetUniform1i(textureName, unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, textureId);
}
