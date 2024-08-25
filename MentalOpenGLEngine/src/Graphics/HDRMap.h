#pragma once

class ShaderProgram;

class HDRMap
{
public:
	~HDRMap();
	void Setup(const char* hdrTexturePath, int envMapSize, int convolutionSize);
	inline unsigned int GetCubeMapTextureId() const { return mEnvCubemap; }
	inline unsigned int GetIrradianceMapTextureId() const { return mIrradianceMap; }
	inline unsigned int GetPrefilterMapTextureId() const { return mPrefilterMap; }

private:
	void SetupCube();
	void DrawCube();
	ShaderProgram SetupEquirectangularToCubemapShader();
	ShaderProgram SetupIrradianceConvolutionShader();
	ShaderProgram SetupHDRPrefilterShader();
	unsigned int SetupCubemap(int width, int height, bool generateMipMap);
	void SetupFramebuffer(int width, int height);
	void RescaleFramebuffer(int width, int height);

	unsigned int mVAO, mVBO;

	unsigned int mEnvCubemap;
	unsigned int mIrradianceMap;
	unsigned int mPrefilterMap;

	unsigned int mCaptureFBO, mCaptureRBO;
};
