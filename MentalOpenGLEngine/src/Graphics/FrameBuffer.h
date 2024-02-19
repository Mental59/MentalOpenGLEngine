#pragma once

class FrameBuffer
{
public:
	virtual ~FrameBuffer();

	void Create(int width, int height);

	void Bind();
	void Unbind();

	inline unsigned int GetTextureColorId() const { return mTextureColorID; }

private:
	unsigned int mFramebufferID;
	unsigned int mTextureColorID;
	unsigned int mDepthStencilRenderBufferID;
};
