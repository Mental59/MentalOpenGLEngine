#pragma once
class GFrameBuffer
{
public:
	virtual ~GFrameBuffer();

	void Create(int width, int height);

	inline unsigned int GetAlbedoSpecularTextureId() const { return mAlbedoSpecularTextureId; }
	inline unsigned int GetMetallicRoughnessAOTextureId() const { return mMetallicRoughnessAOTextureId; }
	inline unsigned int GetNormalTextureId() const { return mNormalTextureId; }
	inline unsigned int GetPositionTextureId() const { return mPositionTextureId; }
	inline unsigned int GetFrameBufferId() const { return mFramebufferId; }

	void Bind();
	void Unbind();

private:
	int mWidth, mHeight;

	unsigned int mFramebufferId;
	unsigned int mAlbedoSpecularTextureId;
	unsigned int mMetallicRoughnessAOTextureId;
	unsigned int mNormalTextureId;
	unsigned int mPositionTextureId;
	unsigned int mRenderBufferId;
};

