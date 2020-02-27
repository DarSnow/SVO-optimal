#include "ShadowRender.h"

#include "../Common/GLheader.h"
#include "../object/light.h"
#include "../Scene/scene.h"

void ShadowMapRenderer::Render()
{
	static Scene * scenePtr = nullptr;
	static auto scene = Scene::getInstance();
	auto camera = scene->camera;

	if (!scene || !shadowCaster)
	{
		std::cout << "shadow render failed\n";
		return;
	}

	// initially assign invalid direction
	static auto &changes = GLTransform::TransformChangedMap();
	static bool updateShadowMap = true;

	// any node transformation happened
	for (auto &c : changes)
	{
		updateShadowMap = true; break;
	}

	// shadow caster change
	if (shadowCaster->TransformChanged()) { updateShadowMap = true; }

	if (!updateShadowMap) { return; }

	updateShadowMap = false;
	// update shadow map

	glBindFramebuffer(GL_FRAMEBUFFER ,shadowFramebuffer);
	glViewport(0, 0, shadowMapSize.x, shadowMapSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// activate geometry pass shader program
	Shader prog = AssetManager::getInstance()->shaderPrograms["depth"];
	// rendering flags
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// unneded since directional light cover the whole scene
	// can be useful for view frustum aware light frustum later
	lightView.DoFrustumCulling(false);
	// scene spatial cues
	auto sceneBB = scene->boundaries;
	auto &center = sceneBB.Center();
	auto radius = distance(center, sceneBB.MaxPoint());
	// fix light frustum to fit scene bounding sphere
	lightView.OrthoRect(glm::vec4(-radius, radius, -radius, radius));
	lightView.ClipPlaneNear(-radius);
	lightView.ClipPlaneFar(2.0f * radius);
	lightView.Projection(Camera::ProjectionMode::Orthographic);
	lightView.transform.Position(center + shadowCaster->Direction() * radius);
	lightView.transform.Forward(-shadowCaster->Direction());
	// update lightview matrix
	LightSpaceMatrix();
	// uniforms
	prog.setVec2("exponents", exponents);
	// draw whole scene tree from root node
	scene->bindMatrices(prog, scene->camera);
	scene->draw(prog);

	// recover
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// no trilinear filtering
	if (filtering < 2) return;

	// mip map shadow map
	glBindTexture(GL_TEXTURE_2D, shadowMap.id);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void ShadowMapRenderer::Caster(const Light * caster)
{
	shadowCaster = caster;
}

const Light * ShadowMapRenderer::Caster() const
{
	return shadowCaster;
}

const glm::mat4x4 &ShadowMapRenderer::LightSpaceMatrix()
{
	static glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0);

	if (lightView.TransformChanged())
	{
		return lightSpaceMatrix = biasMatrix * lightView.ViewProjectionMatrix();
	}

	return lightSpaceMatrix;
}


const Camera &ShadowMapRenderer::LightCamera() const
{
	return lightView;
}

const Texture &ShadowMapRenderer::ShadowMap() const
{
	return shadowMap;
}

ShadowMapRenderer::ShadowMapRenderer() : shadowCaster(nullptr)
{
	filtering = 1;
	exponents = glm::vec2(40.0f, 5.0f);
	lightBleedingReduction = 0.0f;
	SetupFramebuffers(1024, 1024);
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

const glm::vec2 &ShadowMapRenderer::Exponents() const
{
	return exponents;
}

void ShadowMapRenderer::Exponents(const glm::vec2 &val)
{
	exponents = val;
}

const float &ShadowMapRenderer::LightBleedingReduction() const
{
	return lightBleedingReduction;
}

void ShadowMapRenderer::LightBleedingReduction(const float &val)
{
	lightBleedingReduction = val;
}

void ShadowMapRenderer::SetupFramebuffers(const unsigned &w,
	const unsigned &h)
{
	// save size
	shadowMapSize = glm::uvec2(w, h);
	// setup shadow framebuffer
	glGenFramebuffers(1, &shadowFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
	// create render buffer for depth testing
	glGenRenderbuffers(1, &depthRender);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRender);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

	//create texture
	glGenTextures(1, &shadowMap.id);
	glBindTexture(GL_TEXTURE_2D, shadowMap.id);
	// create variance shadow mapping texture, z and z * z
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA32F, GL_FLOAT, nullptr);
	Filtering(filtering);
	Anisotropy(8);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMap.id, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRender);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// check if success building frame buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "ERROR::FRAMEBUFFER:: shadowmap buffer is not complete!" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapRenderer::Anisotropy(const int &val) const
{
	glBindTexture(GL_TEXTURE_2D, shadowMap.id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
}

void ShadowMapRenderer::Filtering(const int &val)
{
	filtering = glm::clamp(val, 0, 2);
	glBindTexture(GL_TEXTURE_2D, shadowMap.id);

	if (filtering == 0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if (filtering == 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (filtering == 2) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}