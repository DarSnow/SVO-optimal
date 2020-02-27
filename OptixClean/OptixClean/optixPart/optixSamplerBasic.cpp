#include "optixSampler.h"


void OptixSampler::setupAllVAO() {
	// setup quad AVO
	static float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// setup sample VAO
	glGenVertexArrays(1, &sampleVAO);
	glGenBuffers(1, &sampleVBO);
	glBindVertexArray(sampleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sampleVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void OptixSampler::drawPoints() {
	glDisable(GL_DEPTH_TEST);
	pointShader->use();

	glBindVertexArray(sampleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sampleVBO);
	float* arr = new float[3000];
	for (int i = 0; i < sampleIdx; ++i) {
		int arridx = i * 6;
		int samidx = i * 4;
		arr[arridx] = samplePosArr[samidx];
		arr[arridx + 1] = samplePosArr[samidx + 1];
		arr[arridx + 2] = samplePosArr[samidx + 2];
		arr[arridx + 3] = sampleClrArr[samidx];
		arr[arridx + 4] = sampleClrArr[samidx + 1];
		arr[arridx + 5] = sampleClrArr[samidx + 2];
		continue;
	}
	glBufferData(GL_ARRAY_BUFFER, sampleIdx * 6 * sizeof(float), arr, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glm::mat4 view = glm::lookAt(glm::vec3(camera_eye.x, camera_eye.y, camera_eye.z),
		glm::vec3(camera_lookat.x, camera_lookat.y, camera_lookat.z),
		glm::vec3(camera_up.x, camera_up.y, camera_up.z));
	glm::mat4 proj = glm::perspective(glm::radians(vfov), static_cast<float>(width) / static_cast<float>(height),
		1.0f, 200.0f);
	pointShader->setMat4("transform", (proj*view));
	pointShader->setInt("sampleGap", samplenum);
	glPointSize(5.5f);
	glDrawArrays(GL_POINTS, 0, sampleIdx);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void OptixSampler::displayBufferToScr(RTbuffer m_buffer, bufferPixelFormat g_image_buffer_format)
{
	optix::Buffer buffer = Buffer::take(m_buffer);

	// Query buffer information
	RTsize buffer_width_rts, buffer_height_rts;
	buffer->getSize(buffer_width_rts, buffer_height_rts);
	uint32_t width = static_cast<int>(buffer_width_rts);
	uint32_t height = static_cast<int>(buffer_height_rts);
	RTformat buffer_format = buffer->getFormat();

	GLboolean use_SRGB = GL_FALSE;
	if (!g_disable_srgb_conversion && (buffer_format == RT_FORMAT_FLOAT4 || buffer_format == RT_FORMAT_FLOAT3))
	{
		glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &use_SRGB);
		if (use_SRGB)
			glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	}

	static unsigned int gl_tex_id = 0;
	if (!gl_tex_id)
	{
		glGenTextures(1, &gl_tex_id);
		glBindTexture(GL_TEXTURE_2D, gl_tex_id);

		// Change these to GL_LINEAR for super- or sub-sampling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gl_tex_id);

	// send PBO or host-mapped image data to texture
	const unsigned pboId = buffer->getGLBOId();
	GLvoid* imageData = 0;
	if (pboId)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
	else
		imageData = buffer->map(0, RT_BUFFER_MAP_READ);

	RTsize elmt_size = buffer->getElementSize();
	if (elmt_size % 8 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	else if (elmt_size % 4 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else if (elmt_size % 2 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	else                          glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLenum pixel_format = glFormatFromBufferFormat(g_image_buffer_format, buffer_format);

	if (buffer_format == RT_FORMAT_UNSIGNED_BYTE4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, imageData);
	else if (buffer_format == RT_FORMAT_FLOAT4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, pixel_format, GL_FLOAT, imageData);
	else if (buffer_format == RT_FORMAT_FLOAT3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width, height, 0, pixel_format, GL_FLOAT, imageData);
	else if (buffer_format == RT_FORMAT_FLOAT)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, width, height, 0, pixel_format, GL_FLOAT, imageData);
	else
		throw Exception("Unknown buffer format");

	if (pboId)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	else
		buffer->unmap();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(quadVAO);
	tex2ScrShader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_tex_id);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_TEXTURE_2D);

	if (use_SRGB)
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);
}


void OptixSampler::getBufferContent(optix::Buffer buffer, bool isSample, bufferPixelFormat format) {
	g_disable_srgb_conversion = false;
	if (isSample) {
		getPNCBufferToArr();
	}
	else {
		displayBufferToScr(buffer->get(), format);
	}
}