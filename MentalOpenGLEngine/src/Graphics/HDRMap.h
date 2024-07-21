#pragma once

class HDRMap
{
public:
	~HDRMap();
	void Setup(const char* hdrTexturePath, int width, int height);
	inline unsigned int GetCubeMapTextureId() const { return mEnvCubemap; }

private:
	void SetupCube();
	void DrawCube();
	class ShaderProgram SetupShader();
	void SetupEnvCubemap(int width, int height);
	void SetupFramebuffer(int width, int height);

	unsigned int mVAO, mVBO;
	unsigned int mEnvCubemap;
	unsigned int mCaptureFBO, mCaptureRBO;
};
