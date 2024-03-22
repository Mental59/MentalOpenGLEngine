#pragma once

class DepthMap
{
public:
	enum DepthMapType
	{
		Directional, Point
	};

	void Build(unsigned int width, unsigned int height, DepthMapType type = Directional);
	void Bind();
	void Unbind();
	virtual ~DepthMap();

	inline unsigned int GetWidth() const { return mWidth; }
	inline unsigned int GetHeight() const { return mHeight; }
	inline unsigned int GetTextureColorId() const { return mDepthMapTexture; }

private:
	unsigned int mWidth, mHeight, mFBO, mDepthMapTexture;
};
