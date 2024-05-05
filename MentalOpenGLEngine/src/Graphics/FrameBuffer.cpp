#include "FrameBuffer.h"
#include <glad/glad.h>
#include <stdio.h>
#include <iostream>

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &mFramebufferID);
	glDeleteTextures(1, &mTextureColorMSAAID);
	glDeleteTextures(1, &mBrightColorMSAAID);
	glDeleteRenderbuffers(1, &mDepthStencilRenderBufferID);

	glDeleteFramebuffers(1, &mIntermediateFramebufferID);
	glDeleteTextures(1, &mTextureColorID);
	glDeleteTextures(1, &mBrightColorID);
}

void FrameBuffer::Create(int width, int height, int samples)
{
	mWidth = width;
	mHeight = height;

	glGenFramebuffers(1, &mFramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);

	glGenTextures(1, &mTextureColorMSAAID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTextureColorMSAAID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA16F, width, height, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mTextureColorMSAAID, 0);

	glGenTextures(1, &mBrightColorMSAAID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mBrightColorMSAAID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA16F, width, height, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, mBrightColorMSAAID, 0);

	glGenRenderbuffers(1, &mDepthStencilRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderBufferID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthStencilRenderBufferID);

	unsigned int attachments[2]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: Framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &mIntermediateFramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, mIntermediateFramebufferID);

	glGenTextures(1, &mTextureColorID);
	glBindTexture(GL_TEXTURE_2D, mTextureColorID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureColorID, 0);

	glGenTextures(1, &mBrightColorID);
	glBindTexture(GL_TEXTURE_2D, mBrightColorID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mBrightColorID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error: Intermediate Framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::Blit()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mIntermediateFramebufferID);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
}

void FrameBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
