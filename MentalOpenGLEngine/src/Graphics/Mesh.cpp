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

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::BindTextures(ShaderProgram& shader)
{
	auto diffuseTexture = mTextures.find(Core::Diffuse);
	auto specularTexture = mTextures.find(Core::Specular);
	auto normalTexture = mTextures.find(Core::Normal);

	if (diffuseTexture != mTextures.end())
	{
		SetTexture(shader, std::format(DIFFUSE_TEXTURE_NAME, 1), 0, diffuseTexture->second.ID);
	}

	if (specularTexture != mTextures.end())
	{
		SetTexture(shader, std::format(SPECULAR_TEXTURE_NAME, 1), 1, specularTexture->second.ID);
		shader.SetUniform1i("uUseSpecularTexture", 1);
	}
	else
	{
		shader.SetUniform1i("uUseSpecularTexture", 0);
	}

	if (normalTexture != mTextures.end())
	{
		SetTexture(shader, std::format(NORMAL_TEXTURE_NAME, 1), 2, normalTexture->second.ID);
		shader.SetUniform1i("uUseNormalTexture", 1);
	}
	else
	{
		shader.SetUniform1i("uUseNormalTexture", 0);
	}

	//for (int i = 0; i < mTextures.size(); i++)
	//{
	//	if (mTextures[i].Type == Core::Diffuse)
	//	{
	//		shader.SetUniform1i(std::format(DIFFUSE_TEXTURE_NAME, diffuseTextureNumber++), i);
	//	}
	//	if (mTextures[i].Type == Core::Specular)
	//	{
	//		shader.SetUniform1i(std::format(SPECULAR_TEXTURE_NAME, specularTextureNumber++), i);
	//	}
	//	if (mTextures[i].Type == Core::Normal)
	//	{
	//		shader.SetUniform1i(std::format(NORMAL_TEXTURE_NAME, normalTextureNumber++), i);
	//	}

	//	glActiveTexture(GL_TEXTURE0 + i);
	//	glBindTexture(GL_TEXTURE_2D, mTextures[i].ID);
	//}
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
