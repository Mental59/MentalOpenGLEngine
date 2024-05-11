#pragma once

class FrameBuffer
{
public:
	virtual ~FrameBuffer();

	void Create(int width, int height);

	void Bind();
	void Unbind();

	inline unsigned int GetTextureColorId() const { return mTextureColorID; }
	inline unsigned int GetBrightTextureColorId() const { return mBrightColorID; }
	inline unsigned int GetFrameBufferId() const { return mFramebufferID; }

private:
	int mWidth, mHeight;

	unsigned int mFramebufferID;
	unsigned int mTextureColorID;
	unsigned int mBrightColorID;
	unsigned int mDepthRenderBufferID;
};
