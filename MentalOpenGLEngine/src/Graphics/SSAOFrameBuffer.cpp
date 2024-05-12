#include "SSAOFrameBuffer.h"
#include <glad/glad.h>
#include <iostream>

SSAOFrameBuffer::~SSAOFrameBuffer()
{
	glDeleteFramebuffers(1, &mFrameBufferId);
	glDeleteTextures(1, &mTextureId);
}

void SSAOFrameBuffer::Create(int width, int height)
{
	glGenFramebuffers(1, &mFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SSAOFrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
}

void SSAOFrameBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
