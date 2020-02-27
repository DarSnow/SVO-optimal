#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
using namespace std;

#include "../Common/GLheader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "../object/MeshObject.h"
#include "../object/camera.h"
#include "../AssetManager/AssetManager.h"
#include "../AssetManager/Loader.h"
#include "../object/light.h"
#include "../Window/myWindow.h"
#include "../Common/ImageTool.h"
#include "../Common/FrameBuffer.h"
#include "../Common/Twin.h"
#include "../Common/FullScreenQuad.h"
#include "../object/BoundingBox.h"
#include "../Renderer/TextRender.h"
#include "../VPLManage/VPL.h"
#include "../Renderer/VoxelRender.h"
#include "../Renderer/ShadowRender.h"
//#include "../PoissonSampleUtil/PoissonSample.h"
using namespace FrameBuffer;
#include "2dScene.h"

class Scene : public Singleton<Scene> {
public:
	Scene() {
		for (int i = 0; i < STATE_NUM; i++) state[i] = false;
	}

	void init();
	void draw(Shader& norshader);
	void drawSkybox(Shader& shader);
	void bindMatrices(Shader& norshader, Camera& cam);
	void run();
	int samplenum;
	int width, height;
	Camera camera;
	BoundingBox boundaries;
private:
	void initLight();
	void initCamera();
	void initSceneObject();
	void makeBoundaries();

	void draw2D();

	void vplCreation(int selectedNum, GLuint* vplDepthMapFBO, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol, float vplPosBias);
	void vplIllumination(int selectedNum, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol, float vplPosBias);

	void animate();

	int getSamplePointAndNor(float* &pos, float* &nor);
	int getSamplePointAndNor(float *& pos, float *& nor, float *& col);
	double calcuLightImportance(vec3 lightpos, vec3 lightdir, int pointnum, float* gpos);

	bool handleInput();
	bool progressMessage();
private:
	void drawModelWithColor(char* modelId);

	enum State {
		W = 0, A = 1, S = 2, D = 3, Q = 4, E = 5, P = 6,
		isOutFileOpen = 7, SPACE = 10,gameStart = 11
	};
	const static int STATE_NUM = 220;
	bool state[STATE_NUM];

	FramebufferDesc tframe;

	GLint maxSupportTex;

	WindowCallBack::MyWindow* window;
	int frameNum;

	Shader *shader, *shaderBox;
	//cell--------------------------<
	//Shader *pMapCreationShader, *pMapUsageShader;
	//cell-------------------------->
	TransCamera spcamera;
	vector<Light> light;

	TextRender* textRender;
	VoxelRender* voxelRenderer;
	ShadowMapRenderer* shadowRenderer;

	friend class VoxelRender;
	friend class OptixSampler;
private:
	map<string, Twin<float>*> twinsmap;
	void setupAnimation();

	//details related
	//func prototype
	void setupDepthFBO(GLuint &depthMapFront, GLuint &depthMapBack, GLuint depthMapFrontFBO, GLuint depthMapBackFBO,
		int selectedNum, GLuint* vplDepthMap, GLuint* vplDepthMapFBO);

	// 顶部光源的depth map分辨率1024，vpl的depth map分辨率256
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const unsigned int VPL_SHADOW_WIDTH = 256, VPL_SHADOW_HEIGHT = 256;

	Shader *pMapCreationShader, *pMapUsageShader, *vplPSMCreationShader, *vplPSMUsageShader, *globalPSMUsageShader,
		*debugDepthQuad, *geometryPassShader, *deferredGlobalPSMUsageShader, *gbufferDebugShader, *cMapCreationShader;

	//顶部光源朝下看
	float light_near_plane = 1.0f;
	float light_far_plane = 25.0f;

	FramebufferDesc gbufferFBO;
	FramebufferDesc lightdepthCubeFBO;

	FullScreenQuad quad;
	GLuint vplVBO, vplVAO;
};
extern bool saveImageFlag;

const glm::vec3 unitY(0.0f, 1.0f, 0.0f);
const glm::vec3 unitZ(0.0f, 0.0f, 1.0f);
const glm::vec3 unitX(1.0f, 0.0f, 0.0f);