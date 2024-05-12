#pragma once

class SSAOFrameBuffer
{
public:
	virtual ~SSAOFrameBuffer();

	void Create(int width, int height);

	void Bind();
	void Unbind();

	inline unsigned int GetTextureColorId() const { return mTextureId; }

private:
	unsigned int mFrameBufferId;
	unsigned int mTextureId;
};
