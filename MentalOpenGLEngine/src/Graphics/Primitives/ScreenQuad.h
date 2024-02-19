#pragma once

class ScreenQuad
{
public:
	virtual ~ScreenQuad();
	void Create();
	void Draw();

private:
	unsigned int mVAO, mVBO;
};
