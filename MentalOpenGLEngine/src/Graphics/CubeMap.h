#pragma once

#include <string>

class CubeMap
{
public:
	virtual ~CubeMap();
	void Load(const char* faces[6]);
	void Load(unsigned int textureId);
	void SetupCube();
	void BindTexture(int textureBlock = 0);
	void UnbindTexture(int textureBlock = 0);
	void Draw();
	inline unsigned int GetTextureID() const { return mTextureId; }
private:
	unsigned int mTextureId, mVAO, mVBO;
};

