#pragma once

class FrameBuffer
{
public:
	virtual ~FrameBuffer();

	void Create(int width, int height, int samples);

	void Bind();
	void Unbind();

	inline unsigned int GetTextureColorId() const { return mTextureColorID; }

private:
	int mWidth, mHeight;

	unsigned int mFramebufferID;
	unsigned int mIntermediateFramebufferID;
	unsigned int mTextureColorMSAAID;
	unsigned int mTextureColorID;
	unsigned int mDepthStencilRenderBufferID;
};
