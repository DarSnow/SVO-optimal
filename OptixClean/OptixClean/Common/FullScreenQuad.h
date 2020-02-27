#pragma once
#include "GLheader.h"

class FullScreenQuad {
public:
	GLuint quadVAO, quadVBO;

	static float quadVertices[];

	void Load();
	void Draw() const;

	~FullScreenQuad();
};