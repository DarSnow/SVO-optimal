#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Common/GLheader.h"
#include "../Common/FrameBuffer.h"
#include "../ModelClass/Shader.h"

#include <vector>

class CubeRender {
public:
	static void CubeMapPrepare(ModelClass::Shader* cubeshader, FrameBuffer::FramebufferDesc &cubeFBO, glm::vec3 pos, glm::vec2 shadowsz, glm::vec2 farnearplane) 
	{
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)shadowsz.x / (float)shadowsz.y, farnearplane.x, farnearplane.y);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		// render scene to depth cubemap
		// --------------------------------
		glViewport(0, 0, shadowsz.x, shadowsz.y);
		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBO.m_nFramebufferId);
		glClear(GL_DEPTH_BUFFER_BIT);
		cubeshader->use();
		for (unsigned int i = 0; i < 6; ++i)
			cubeshader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		cubeshader->setVec3("lightPos", pos);
		cubeshader->setFloat("far_plane", farnearplane.y);
	}
};