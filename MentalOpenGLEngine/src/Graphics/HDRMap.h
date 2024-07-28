#pragma once

class HDRMap
{
public:
	~HDRMap();
	void Setup(const char* hdrTexturePath, int width, int height, int convolutionWidth, int convolutionHeight);
	inline unsigned int GetCubeMapTextureId() const { return mEnvCubemap; }
	inline unsigned int GetIrradianceMapTextureId() const { return mIrradianceMap; }

private:
	void SetupCube();
	void DrawCube();
	class ShaderProgram SetupEquirectangularToCubemapShader();
	class ShaderProgram SetupIrradianceConvolutionShader();
	unsigned int SetupCubemap(int width, int height);
	void SetupFramebuffer(int width, int height);
	void RescaleFramebuffer(int width, int height);

	unsigned int mVAO, mVBO;
	unsigned int mEnvCubemap;
	unsigned int mIrradianceMap;
	unsigned int mCaptureFBO, mCaptureRBO;
};
