#pragma once
#ifndef  FRAMEBUFFER_DEF
#define FRAMEBUFFER_DEF

#include "GLheader.h"

namespace FrameBuffer {
	//Ö¡»º´æ
	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nTexture0Id;
		GLuint m_nTexture1Id;
		GLuint m_nFramebufferId;
		GLuint m_pos, m_nor, m_albedo;
		int width;
		int height;
	};

	static bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
	{
		framebufferDesc.width = nWidth;
		framebufferDesc.height = nHeight;

		glGenFramebuffers(1, &framebufferDesc.m_nFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nFramebufferId);
		// create a color attachment texture

		glGenTextures(1, &framebufferDesc.m_nTexture0Id);
		glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nTexture0Id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nTexture0Id, 0);
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

		glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, nWidth, nHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	static bool CreateGBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
	{
		framebufferDesc.width = nWidth;
		framebufferDesc.height = nHeight;

		glGenFramebuffers(1, &framebufferDesc.m_nFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nFramebufferId);
		// create a color attachment texture

		glGenTextures(1, &framebufferDesc.m_pos);
		glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_pos);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, nWidth, nHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_pos, 0);

		glGenTextures(1, &framebufferDesc.m_nor);
		glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, nWidth, nHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferDesc.m_nor, 0);

		glGenTextures(1, &framebufferDesc.m_albedo);
		glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_albedo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nWidth, nHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, framebufferDesc.m_albedo, 0);

		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
		
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, nWidth, nHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			cout << "ERROR::FRAMEBUFFER:: Gbuffer is not complete!" << endl;
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	static bool CreateCubemapFramebuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc) {
		glGenFramebuffers(1, &framebufferDesc.m_nFramebufferId);
		glGenTextures(1, &framebufferDesc.m_nTexture0Id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, framebufferDesc.m_nTexture0Id);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nFramebufferId);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebufferDesc.m_nTexture0Id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			cout << "ERROR::FRAMEBUFFER:: CubemapFramebuffer is not complete!" << endl;
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	static bool CreateVelocityFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
	{
		framebufferDesc.width = nWidth;
		framebufferDesc.height = nHeight;

		glGenFramebuffers(1, &framebufferDesc.m_nFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nFramebufferId);
		// create a color attachment texture

		glGenTextures(1, &framebufferDesc.m_nTexture0Id);
		glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nTexture0Id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, nWidth, nHeight, 0, GL_RG, GL_HALF_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nTexture0Id, 0);

		glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, nWidth, nHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			cout << "ERROR::FRAMEBUFFER:: VelocityFramebuffer is not complete!" << endl;
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	static bool CreateMutiSampledFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
	{
		framebufferDesc.width = nWidth;
		framebufferDesc.height = nHeight;

		glGenFramebuffers(1, &framebufferDesc.m_nFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nFramebufferId);
		// create a color attachment texture

		glGenTextures(1, &framebufferDesc.m_nTexture0Id);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nTexture0Id);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, nWidth, nHeight, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nTexture0Id, 0);

		/*glGenTextures(1, &framebufferDesc.m_nDepthBufferId);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nDepthBufferId);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT32, nWidth, nHeight, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nDepthBufferId, 0);
		*/
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32, nWidth, nHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			cout << "ERROR::FRAMEBUFFER:: MSAA Framebuffer is not complete!" << endl;
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	static void DeleteFrameBuffer(FramebufferDesc &framebufferDesc) {
		glDeleteTextures(1, &framebufferDesc.m_nDepthBufferId);
		glDeleteTextures(1, &framebufferDesc.m_nTexture1Id);
		glDeleteTextures(1, &framebufferDesc.m_nTexture0Id);
		glDeleteFramebuffers(1, &framebufferDesc.m_nFramebufferId);
	}
}
#endif // ! FRAMEBUFFER_DEF