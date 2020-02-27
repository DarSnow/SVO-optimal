#pragma once

#include "../object/camera.h"
#include "../ModelClass/Texture.h"

class BlurProgram;
class DepthProgram;
class Light;
using namespace ModelClass;

class ShadowMapRenderer
{
public:
	void Render();
	void Caster(const Light * caster);
	const glm::mat4x4 &LightSpaceMatrix();
	const Light * Caster() const;
	void SetupFramebuffers(const unsigned &w, const unsigned &h);

	void Anisotropy(const int &val) const;
	void Filtering(const int &val);
	const float &LightBleedingReduction() const;
	void LightBleedingReduction(const float &val);
	const glm::vec2 &Exponents() const;
	void Exponents(const glm::vec2 &val);

	const Camera &LightCamera() const;
	const Texture &ShadowMap() const;
	explicit ShadowMapRenderer();
	~ShadowMapRenderer();
private:
	GLuint shadowFramebuffer;
	GLuint depthRender;
	Texture shadowMap;
	Texture blurShadow;

	glm::uvec2 shadowMapSize;
	glm::vec2 exponents;
	float lightBleedingReduction;
	Camera lightView;
	const Light * shadowCaster;
	glm::mat4x4 lightSpaceMatrix;
	int filtering;
};

