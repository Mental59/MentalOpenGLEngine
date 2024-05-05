#pragma once

class FrameBuffer
{
public:
	virtual ~FrameBuffer();

	void Create(int width, int height, int samples);
	void Blit();

	void Bind();
	void Unbind();

	inline unsigned int GetTextureColorId() const { return mTextureColorID; }
	inline unsigned int GetBrightTextureColorId() const { return mBrightColorID; }

private:
	int mWidth, mHeight;

	unsigned int mFramebufferID;
	unsigned int mIntermediateFramebufferID;
	unsigned int mTextureColorMSAAID;
	unsigned int mBrightColorMSAAID;
	unsigned int mTextureColorID;
	unsigned int mBrightColorID;
	unsigned int mDepthStencilRenderBufferID;
};
