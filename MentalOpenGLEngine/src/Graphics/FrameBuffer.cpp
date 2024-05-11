#include "FrameBuffer.h"
#include <glad/glad.h>
#include <stdio.h>
#include <iostream>

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &mFramebufferID);
	glDeleteRenderbuffers(1, &mDepthRenderBufferID);
	glDeleteTextures(1, &mTextureColorID);
	glDeleteTextures(1, &mBrightColorID);
}

void FrameBuffer::Create(int width, int height)
{
	glGenFramebuffers(1, &mFramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);

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

	glGenRenderbuffers(1, &mDepthRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderBufferID);

	unsigned int attachments[2]{
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, attachments);

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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
