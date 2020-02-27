#pragma once

#include "../Common/FrameBuffer.h"
#include "../AssetManager/AssetManager.h"
#include "../ModelClass/Shader.h"
#include "../object/camera.h"
#include "../Scene/scene.h"

class GBufferRenderer {
public:
	GBufferRenderer(int w, int h) {
		FrameBuffer::CreateGBuffer(w, h, gbuffer);
		prog = AssetManager::getInstance()->shaderPrograms["gbuffer"];
	}

	void Render(Camera& cam) {
		prog.use();
		static auto scene = Scene::getInstance();
		scene->bindMatrices(prog, cam);
		scene->draw(prog);
	}

	FrameBuffer::FramebufferDesc Gbuffer() const {
		return gbuffer;
	}

private:
	FrameBuffer::FramebufferDesc gbuffer;
	ModelClass::Shader prog;
};