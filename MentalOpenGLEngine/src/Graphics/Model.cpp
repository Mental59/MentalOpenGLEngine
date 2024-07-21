#include "Model.h"

#include <iostream>
#include <format>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Graphics/Utils.h"

Model::~Model()
{
	for (const auto& loadedTextures : mLoadedTextures)
	{
		glDeleteTextures(1, &loadedTextures.second);
	}
	glDeleteBuffers(1, &mInstanceMatrixVBO);
}

Model::Model(bool flipTexturesVertically) :
	mFlipTexturesVertically(flipTexturesVertically), mInstanceMatrixVBO(0u)
{

}

void Model::Draw(ShaderProgram& shader)
{
	glm::mat4 modelMat(1.0f);
	modelMat = glm::translate(modelMat, mTransform.Position);
	modelMat = glm::rotate(modelMat, glm::radians(mTransform.RotationAngle), mTransform.RotationAxis);
	modelMat = glm::scale(modelMat, mTransform.Scale);

	shader.SetUniformMat4("uModel", glm::value_ptr(modelMat));

	for (size_t i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->Draw(shader);
	}
}

void Model::Draw(ShaderProgram& shader, const Core::Transform& transform)
{
	glm::mat4 modelMat(1.0f);
	modelMat = glm::translate(modelMat, transform.Position);
	modelMat = glm::rotate(modelMat, glm::radians(transform.RotationAngle), transform.RotationAxis);
	modelMat = glm::scale(modelMat, transform.Scale);

	shader.SetUniformMat4("uModel", glm::value_ptr(modelMat));

	for (size_t i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->Draw(shader);
	}
}

void Model::Draw(ShaderProgram& shader, const glm::mat4& modelMat)
{
	shader.SetUniformMat4("uModel", glm::value_ptr(modelMat));

	for (size_t i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->Draw(shader);
	}
}

void Model::DrawInstanced(ShaderProgram& shader, int n)
{
	for (size_t i = 0; i < mMeshes.size(); i++)
	{
		mMeshes[i]->DrawInstanced(shader, n);
	}
}

bool Model::HasTexture(Core::TextureType type) const
{
	return false;
}

void Model::SetDefaultTexture(const Core::Texture& texture)
{
	auto it = mDefaultTextures.find(texture.Type);
	if (it == mDefaultTextures.end())
	{
		mDefaultTextures[texture.Type] = texture;
	}
	else
	{
		it->second = texture;
	}
}

void Model::SetTransform(const Core::Transform& transform)
{
	mTransform = transform;
}

bool Model::HasDefaultTexture(Core::TextureType textureType) const
{
	return mDefaultTextures.find(textureType) != mDefaultTextures.end();
}

void Model::SetupInstancedDrawing(glm::mat4* instanceMatrices, size_t size, unsigned int location)
{
	glGenBuffers(1, &mInstanceMatrixVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mInstanceMatrixVBO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::mat4), instanceMatrices, GL_STATIC_DRAW);

	for (size_t i = 0; i < mMeshes.size(); i++)
	{
		unsigned int VAO = mMeshes[i]->GetVAO();
		glBindVertexArray(VAO);

		for (unsigned int j = 0; j < 4; j++)
		{
			glEnableVertexAttribArray(location + j);
			glVertexAttribPointer(location + j, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(j * sizeof(glm::vec4)));
			glVertexAttribDivisor(location + j, 1);
		}
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::Load(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << std::format("Assimp: An error occurred while loading the model: {} \n", importer.GetErrorString());
		return;
	}

	mDirectory = path.substr(0, path.find_last_of('/'));

	mMeshes.reserve(scene->mNumMeshes);
	ProcessNode(scene->mRootNode, scene);

	std::cout << "Assimp: Loaded model " << path << std::endl;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene, mesh);
		mMeshes.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::shared_ptr<Mesh> resMesh)
{
	std::vector<Core::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Core::Texture> textures;

	vertices.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Core::Vertex vertex{
			glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
			glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
			glm::vec2(0.0f)
		};

		if (mesh->mTextureCoords[0])
		{
			vertex.TextureCoordinates = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			vertex.Tangent = glm::vec3(
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z
			);
		}

		vertices.push_back(vertex);
	}

	indices.reserve(mesh->mNumFaces * 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		assert(mesh->mFaces[i].mNumIndices == 3);
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Core::Texture> albedoMaps = LoadMaterialTextures(material, aiTextureType_BASE_COLOR, Core::Albedo);
		std::vector<Core::Texture> metallicMaps = LoadMaterialTextures(material, aiTextureType_METALNESS, Core::Metallic);
		std::vector<Core::Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, Core::Roughness);
		std::vector<Core::Texture> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, Core::AmbientOcclusion);

		std::vector<Core::Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, Core::Normal);
		std::vector<Core::Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, Core::Height);

		if (albedoMaps.size() == 0) AddDefaultTexture(&albedoMaps, Core::Albedo);
		if (metallicMaps.size() == 0) AddDefaultTexture(&metallicMaps, Core::Metallic);
		if (roughnessMaps.size() == 0) AddDefaultTexture(&roughnessMaps, Core::Roughness);
		if (aoMaps.size() == 0) AddDefaultTexture(&aoMaps, Core::AmbientOcclusion);

		if (normalMaps.size() == 0) AddDefaultTexture(&normalMaps, Core::Normal);
		if (heightMaps.size() == 0) AddDefaultTexture(&heightMaps, Core::Height);

		textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
		textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
		textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());

		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}

	resMesh->Setup(vertices, indices, textures);
}

void Model::AddDefaultTexture(std::vector<Core::Texture>* textures, Core::TextureType textureType)
{
	auto it = mDefaultTextures.find(textureType);
	if (it != mDefaultTextures.end())
	{
		textures->push_back(it->second);
	}
}

std::vector<Core::Texture> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType textureType, Core::TextureType coreTextureType)
{
	std::vector<Core::Texture> textures;
	bool srgb = coreTextureType == Core::Albedo;

	for (unsigned int i = 0; i < material->GetTextureCount(textureType); i++)
	{
		aiString textureFilename;
		material->GetTexture(textureType, i, &textureFilename);

		std::string textureFilenameString(textureFilename.C_Str());
		unsigned int textureId = 0;

		if (mLoadedTextures.find(textureFilenameString) != mLoadedTextures.end())
		{
			textureId = mLoadedTextures[textureFilenameString];
		}
		else
		{
			textureId = GLLoadTextureFromFile(std::format("{}/{}", mDirectory, textureFilenameString).c_str(), mFlipTexturesVertically, srgb);
			std::cout << "Assimp: Loaded texture " << textureFilenameString << std::endl;
			mLoadedTextures[textureFilenameString] = textureId;
		}

		textures.push_back({ textureId, coreTextureType });
	}

	return textures;
}
