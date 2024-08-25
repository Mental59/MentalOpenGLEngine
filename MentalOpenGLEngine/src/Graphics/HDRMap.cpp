#include "HDRMap.h"
#include "Utils.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include <glm/gtc/type_ptr.hpp>
#include "Graphics/Primitives/ScreenQuad.h"

HDRMap::~HDRMap()
{
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);

	glDeleteFramebuffers(1, &mCaptureFBO);
	glDeleteRenderbuffers(1, &mCaptureRBO);

	glDeleteTextures(1, &mEnvCubemap);
	glDeleteTextures(1, &mIrradianceMap);
	glDeleteTextures(1, &mPrefilterMap);
}

void HDRMap::Setup(const char* hdrTexturePath, int envMapSize, int convolutionSize)
{
	constexpr unsigned int numViews = 6;
	constexpr unsigned int prefilterMapSize = 128;
	constexpr unsigned int brdfLutTextureSize = 512;

	unsigned int hdrTexture = GLLoadHDRFromFile(hdrTexturePath, true);

	ShaderProgram equirectangularToCubemapShaderProgram = SetupEquirectangularToCubemapShader();
	ShaderProgram irradianceConvolutionShaderProgram = SetupIrradianceConvolutionShader();
	ShaderProgram prefilterShaderProgram = SetupHDRPrefilterShader();
	ShaderProgram brdfPrecomputingShaderProgram = SetupBRDFPrecomputingShader();
	SetupCube();
	SetupFramebuffer(envMapSize, envMapSize);
	mEnvCubemap = SetupCubemap(envMapSize, envMapSize, false);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	mIrradianceMap = SetupCubemap(convolutionSize, convolutionSize, false);
	mPrefilterMap = SetupCubemap(prefilterMapSize, prefilterMapSize, true);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[numViews] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f))
	};

	equirectangularToCubemapShaderProgram.Bind();
	equirectangularToCubemapShaderProgram.SetUniform1i("equirectangularMap", 0);
	equirectangularToCubemapShaderProgram.SetUniform1f("uGamma", 1.0f);
	equirectangularToCubemapShaderProgram.SetUniform1f("uExposure", 1.0f);
	equirectangularToCubemapShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	glViewport(0, 0, envMapSize, envMapSize);
	for (int i = 0; i < numViews; i++)
	{
		equirectangularToCubemapShaderProgram.SetUniformMat4("uView", glm::value_ptr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mEnvCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawCube();
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	RescaleFramebuffer(convolutionSize, convolutionSize);
	irradianceConvolutionShaderProgram.Bind();
	irradianceConvolutionShaderProgram.SetUniform1i("uEnvironmentMap", 0);
	irradianceConvolutionShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvCubemap);
	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	glViewport(0, 0, convolutionSize, convolutionSize);
	for (int i = 0; i < numViews; i++)
	{
		irradianceConvolutionShaderProgram.SetUniformMat4("uView", glm::value_ptr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mIrradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawCube();
	}

	prefilterShaderProgram.Bind();
	prefilterShaderProgram.SetUniform1i("environmentMap", 0);
	prefilterShaderProgram.SetUniform1f("environmentMapSize", envMapSize);
	prefilterShaderProgram.SetUniformMat4("uProjection", glm::value_ptr(captureProjection));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; mipLevel++)
	{
		unsigned int mipWidth = prefilterMapSize * std::pow(0.5, mipLevel);
		unsigned int mipHeight = prefilterMapSize * std::pow(0.5, mipLevel);

		glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1);
		prefilterShaderProgram.SetUniform1f("roughness", roughness);
		for (unsigned int i = 0; i < numViews; i++)
		{
			prefilterShaderProgram.SetUniformMat4("uView", glm::value_ptr(captureViews[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mPrefilterMap, mipLevel);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			DrawCube();
		}
	}

	glGenTextures(1, &mBrdfLutTexture);
	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, mBrdfLutTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfLutTextureSize, brdfLutTextureSize, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdfLutTextureSize, brdfLutTextureSize);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBrdfLutTexture, 0);

	glViewport(0, 0, brdfLutTextureSize, brdfLutTextureSize);
	brdfPrecomputingShaderProgram.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ScreenQuad screenQuad;
	screenQuad.Create();
	glDisable(GL_DEPTH_TEST);
	screenQuad.Draw();
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteTextures(1, &hdrTexture);
}

void HDRMap::SetupCube()
{
	GLfloat cubeVertices[] = {
		//right side
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		//left side
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		// front side
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		// back side
	   -1.0f, -1.0f,  1.0f,
	   -1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f,  1.0f,

	   // upper side
	   -1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f, -1.0f,

	   // bottom side
	   -1.0f, -1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void HDRMap::DrawCube()
{
	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

ShaderProgram HDRMap::SetupEquirectangularToCubemapShader()
{
	Shader equirectangularToCubemapVertShader("src/Shaders/equirectangularToCubemap.vert", Shader::Vertex);
	Shader equirectangularToCubemapFragShader("src/Shaders/equirectangularToCubemap.frag", Shader::Fragment);

	ShaderProgram equirectangularToCubemapShaderProgram;
	equirectangularToCubemapShaderProgram.Build({ equirectangularToCubemapVertShader, equirectangularToCubemapFragShader });

	return equirectangularToCubemapShaderProgram;
}

ShaderProgram HDRMap::SetupIrradianceConvolutionShader()
{
	Shader irradianceConvolutionVertShader("src/Shaders/equirectangularToCubemap.vert", Shader::Vertex);
	Shader irradianceConvolutionFragShader("src/Shaders/irradianceConvolution.frag", Shader::Fragment);

	ShaderProgram irradianceConvolutionShaderProgram;
	irradianceConvolutionShaderProgram.Build({ irradianceConvolutionVertShader, irradianceConvolutionFragShader });

	return irradianceConvolutionShaderProgram;
}

ShaderProgram HDRMap::SetupHDRPrefilterShader()
{
	Shader vertShader("src/Shaders/equirectangularToCubemap.vert", Shader::Vertex);
	Shader fragShader("src/Shaders/hdrPrefilter.frag", Shader::Fragment);

	ShaderProgram shaderProgram;
	shaderProgram.Build({ vertShader, fragShader });

	return shaderProgram;
}

ShaderProgram HDRMap::SetupBRDFPrecomputingShader()
{
	Shader vertShader("src/Shaders/brdfPrecomputing.vert", Shader::Vertex);
	Shader fragShader("src/Shaders/brdfPrecomputing.frag", Shader::Fragment);

	ShaderProgram shaderProgram;
	shaderProgram.Build({ vertShader, fragShader });

	return shaderProgram;
}

unsigned int HDRMap::SetupCubemap(int width, int height, bool generateMipMap)
{
	unsigned int cubeMap;

	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (generateMipMap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	return cubeMap;
}

void HDRMap::SetupFramebuffer(int width, int height)
{
	glGenFramebuffers(1, &mCaptureFBO);
	glGenRenderbuffers(1, &mCaptureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mCaptureRBO);
}

void HDRMap::RescaleFramebuffer(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
}
