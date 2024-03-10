#include "FrameBuffer.h"
#include <glad/glad.h>
#include <stdio.h>
#include <iostream>

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &mFramebufferID);
	glDeleteTextures(1, &mTextureColorMSAAID);
	glDeleteRenderbuffers(1, &mDepthStencilRenderBufferID);
}

void FrameBuffer::Create(int width, int height, int samples)
{
	mWidth = width;
	mHeight = height;

	glGenFramebuffers(1, &mFramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);

	glGenTextures(1, &mTextureColorMSAAID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTextureColorMSAAID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mTextureColorMSAAID, 0);

	glGenRenderbuffers(1, &mDepthStencilRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderBufferID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRenderBufferID);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: Framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &mIntermediateFramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, mIntermediateFramebufferID);
	glGenTextures(1, &mTextureColorID);
	glBindTexture(GL_TEXTURE_2D, mTextureColorID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureColorID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: Intermediate Framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
}

void FrameBuffer::Unbind()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mIntermediateFramebufferID);

	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
