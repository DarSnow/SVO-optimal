//#include "../optixPart/optixSampler.h"
//#include "scene.h"
//#include "sampleAssistance.h"
//#include "../Renderer/CubeRender.h"
//#include "../Common/StringTool.h"
//
//
//
////#define GBUFFER
////#define POINTS_DEBUG
//#define VPL_ILLUMINATION
////输出FPS信息
//bool FPS_TEST = 1;
////unsigned int quadVAO;
//float quadVertices[] = {
//	// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
//	// positions   // texCoords
//	-1.0f,  1.0f,  0.0f, 1.0f,
//	-1.0f, -1.0f,  0.0f, 0.0f,
//	1.0f, -1.0f,  1.0f, 0.0f,
//
//	-1.0f,  1.0f,  0.0f, 1.0f,
//	1.0f, -1.0f,  1.0f, 0.0f,
//	1.0f,  1.0f,  1.0f, 1.0f
//};
//
//bool saveImageFlag;
//bool shadows = true;
//float resizeRate = 1;
//
//
////cell-------------------------->
//
//void Scene::init() {
//
//	Loader loader;
//	loader.load("config.txt");
//
//	width = 900;
//	height = 900;
//	window = WindowCallBack::MyWindow::InitWindow(width, height, "driver");
//	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
//	glewExperimental = GL_TRUE;
//	// Initialize GLEW to setup the OpenGL Function pointers
//	GLenum err = glewInit();
//	if (err != GLEW_OK) {
//		fprintf(stderr, "%s\n", glewGetErrorString(err));
//		return;
//	}
//
//	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxSupportTex);
//	AssetManager *am = AssetManager::getInstance();
//	ObjectManager *om = ObjectManager::getInstance();
//
//	am->readConfig(loader.stringMap["model"] + loader.stringMap["shader"] + "end\n");
//	om->readConfig(loader.stringMap["model"] + "end\n");
//	shader = &(am->shaderPrograms["roll"]);
//	shaderBox = &(am->shaderPrograms["box"]);
//
//	//cell--------------------------<
//	//pMapCreationShader = &(am->shaderPrograms["pMapCreationShader"]);
//	//pMapUsageShader = &(am->shaderPrograms["pMapUsageShader"]);
//	//cell-------------------------->
//	light.pos = vec3(0.0, 5.3, 6.8);
//	light.up = glm::zero<vec3>() - unitZ;
//	light.dir = glm::zero<vec3>() - unitY;
//
//	//light.pos = vec3(0.0, 0.0, 0.0);
//	light.ambient = vec3(0.1, 0.2, 0.3);
//
//	//cell--------------------------<
//	//把自带的quad设置取消了，因为和我的renderQuad不共存
//
//	// screen quad VAO
//	unsigned int quadVBO;
//	glGenVertexArrays(1, &quadVAO);
//	glGenBuffers(1, &quadVBO);
//	glBindVertexArray(quadVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
//	//cell-------------------------->
//
//	Shader screenSdr = AssetManager::getInstance()->shaderPrograms["screenshader"];
//	screenSdr.use();
//	screenSdr.setInt("screenTexture", 0);
//
//	pMapCreationShader = &AssetManager::getInstance()->shaderPrograms["pMapCreationShader"];
//	pMapUsageShader = &AssetManager::getInstance()->shaderPrograms["pMapUsageShader"];
//	vplPSMCreationShader = &AssetManager::getInstance()->shaderPrograms["vplPSMCreationShader"];
//	vplPSMUsageShader = &AssetManager::getInstance()->shaderPrograms["vplPSMUsageShader"];
//	// globalPSMUsageShader渲染顶部光源以及vpl
//	globalPSMUsageShader = &AssetManager::getInstance()->shaderPrograms["globalPSMUsageShader"];
//	// 测试depth map
//	debugDepthQuad = &AssetManager::getInstance()->shaderPrograms["debugDepthQuad"];
//	debugDepthQuad->use();
//	debugDepthQuad->setInt("depthMap", 0);
//	// 写入gbuffer
//	geometryPassShader = &AssetManager::getInstance()->shaderPrograms["geometryPassShader"];
//	// 利用gbuffer的光源计算
//	deferredGlobalPSMUsageShader = &AssetManager::getInstance()->shaderPrograms["deferredGlobalPSMUsageShader"];
//	deferredGlobalPSMUsageShader->use();
//	deferredGlobalPSMUsageShader->setInt("gPosition", 0);
//	deferredGlobalPSMUsageShader->setInt("gNormal", 1);
//	deferredGlobalPSMUsageShader->setInt("gAlbedoSpec", 2);
//	//cell-------------------------->
//	// 测试gbuffer的shader
//	gbufferDebugShader = &AssetManager::getInstance()->shaderPrograms["gbufferDebugShader"];
//	// 创建顶部单光源的六面cube map
//	cMapCreationShader = &AssetManager::getInstance()->shaderPrograms["cMapCreationShader"];
//
//	glUseProgram(0);
//
//	CreateFrameBuffer(width, height, tframe);
//	CreateGBuffer(width, height, gbufferFBO);
//	CreateCubemapFramebuffer(SHADOW_WIDTH, SHADOW_HEIGHT, lightdepthCubeFBO);
//
//	frameNum = 0;
//
//	ObjectManager::getInstance()->objects["floor"].scale(vec3(2, 2, 2));
//	ObjectManager::getInstance()->objects["cube"].scale(vec3(0.01, 0.01, 0.01));
//	ObjectManager::getInstance()->objects["cube"].setPos(light.pos);
//	ObjectManager::getInstance()->objects["cornellbox"].scale(vec3(0.1, 0.1, 0.1));
//	//ObjectManager::getInstance()->objects["cornellbox"].
//	ObjectManager::getInstance()->objects["cornellbox"].translate(vec3(0, 1, 7));
//	//ObjectManager::getInstance()->objects["cornellbox"].translate(vec3(0, -4, 7));
//	ObjectManager::getInstance()->objects["cornellbox"].rotate(-glm::pi<float>() / 2, glm::vec3(0, 1, 0));
//	ObjectManager::getInstance()->objects["cornellbox"].rotate(-glm::pi<float>(), glm::vec3(1, 0, 0));
//	//Object* obj = &(ObjectManager::getInstance()->objects["cornellbox"]);
//
//	camera.setFOV(90);
//	// farClip默认500
//	camera.setNearClip(0.3f);
//	camera.pos = vec3(0, 1, -2);
//	camera.dir = ObjectManager::getInstance()->objects["cornellbox"].pos - camera.pos;
//	camera.update();
//	spcamera.updateView(camera.getViewMatrix());
//
//	setupAnimation();
//	textRender = new TextRender(width, height);
//}
//
//float selectedSamplePointsPos[3000];
//float selectedSamplePointsNor[3000];
//float selectedSamplePointsCol[3000];
//
//void Scene::run() {
//
//	//glDisable(GL_CULL_FACE);
//	//float *gPos = new float[gbufferWidth * gbufferHeight * 3];
//	float gPos[30 * 30 * 3];
//	float *samplePointsPos = nullptr, *samplePointsNor = nullptr;
//	float *samplePointsCol = nullptr;
//
//	//samplePointsPos采样点的位置，samplePointsNor采样点的法线
//	int vplNum = getSamplePointAndNor(samplePointsPos, samplePointsNor, samplePointsCol);
//
//	//cell--------------------------<
//	//统计现在vpl数量
//	//makeStatistics(vplNum, samplePointsPos);
//
//	//随机取50个不在地板上的vpl
//	//float selectedVPL[50];
//	vector<int> selectedVPL;
//	GLuint selectedNum = 50;
//	sampleFromeWall(selectedNum, vplNum, selectedVPL, samplePointsPos, samplePointsNor, samplePointsCol,
//		selectedSamplePointsPos, selectedSamplePointsNor, selectedSamplePointsCol);
//
//	//接下来，在这些选择出的vpl点上做抛物面深度map
//	//如何剔除光源不可见的点
//	//做顶部光源的depth map，在渲染点的时候，根据传入的map判断深度，即阴影的原理
//
//	//cell--------------------------<
//	glEnable(GL_DEPTH_TEST);
//	// create depth front texture
//
//
//	GLuint depthMapFront, depthMapBack;
//	GLuint *vplDepthMap = new GLuint[selectedNum];
//
//	// create depth front texture
//	GLuint depthMapFrontFBO;
//	glGenFramebuffers(1, &depthMapFrontFBO);
//	GLuint depthMapBackFBO;
//	glGenFramebuffers(1, &depthMapBackFBO);
//	// create vpl depth texture
//	GLuint *vplDepthMapFBO = new unsigned int[selectedNum];
//	for (int i = 0; i < selectedNum; i++)
//	{
//		glGenFramebuffers(1, &vplDepthMapFBO[i]);
//	}
//
//	setupDepthFBO(depthMapFront, depthMapBack, depthMapFrontFBO, depthMapBackFBO, selectedNum, vplDepthMap, vplDepthMapFBO);
//
//	glGenVertexArrays(1, &vplVAO);
//	glGenBuffers(1, &vplVBO);
//
//	Shader screenSdr = AssetManager::getInstance()->shaderPrograms["screenshader"];
//
//	samplenum = selectedNum;
//	VPL* vplarr = new VPL[samplenum];
//	int nbframes = 0;
//	std::srand(0);
//	double lasttime = glfwGetTime();
//	string timetxt("FPS:");
//
//	int stride = 4;
//	for (int i = 0; i < selectedNum; ++i) {
//		vplarr[i] = VPL(make_float3(selectedSamplePointsPos[stride * i], selectedSamplePointsPos[stride * i + 1], selectedSamplePointsPos[stride * i + 2]),
//			make_float3(selectedSamplePointsNor[stride * i], selectedSamplePointsNor[stride * i + 1], selectedSamplePointsNor[stride * i + 2]),
//			make_float3(selectedSamplePointsCol[stride * i], selectedSamplePointsCol[stride * i + 1], selectedSamplePointsCol[stride * i + 2]));
//	}
//	
//
//	//FPS_TEST = 0;
//	while (!glfwWindowShouldClose(window->window))
//	{
//		// move light position over time
//		//light.pos.y = sin(glfwGetTime() * 0.5) * 3.0 + 2;
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		if (!handleInput())
//			break;
//		animate();
//		
//		/*
//#pragma region VPL_RENDERING
//		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFrontFBO);
//		glEnable(GL_DEPTH_TEST);
//		glClearColor(0.0f, 0.0f, 0.0f, 1.0);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		// 先测试z轴负向面的frontmap
//		pMapCreationShader->use();
//		pMapCreationShader->setMat4("lightView", light.getViewMatrix());
//		pMapCreationShader->setFloat("near_plane", light_near_plane);
//		pMapCreationShader->setFloat("far_plane", light_far_plane);
//		pMapCreationShader->setFloat("direction", 1);
//		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
//		glClear(GL_DEPTH_BUFFER_BIT);
//		//glActiveTexture(GL_TEXTURE0);
//		//glActiveTexture(GL_TEXTURE1);
//		//glBindTexture(GL_TEXTURE_2D, woodTexture);
//		ObjectManager::getInstance()->drawObject("cornellbox", pMapCreationShader->GetProgram());
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//		// 再测试z轴正向面的backmap
//		pMapCreationShader->use();
//		pMapCreationShader->setMat4("lightView", light.getViewMatrix());
//		pMapCreationShader->setFloat("near_plane", light_near_plane);
//		pMapCreationShader->setFloat("far_plane", light_far_plane);
//		pMapCreationShader->setFloat("direction", -1);
//		glBindFramebuffer(GL_FRAMEBUFFER, depthMapBackFBO);
//		glClear(GL_DEPTH_BUFFER_BIT);
//		ObjectManager::getInstance()->drawObject("cornellbox", pMapCreationShader->GetProgram());
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//		// 顶部单光源的cube map生成
//		CubeRender::CubeMapPrepare(cMapCreationShader, lightdepthCubeFBO, light.pos,
//			vec2(SHADOW_WIDTH, SHADOW_HEIGHT), vec2(light_near_plane, light_far_plane));
//		ObjectManager::getInstance()->drawObject("cornellbox", cMapCreationShader->GetProgram());
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//		//cell--------------------------<
//
//		//以下是vpl的PSM创建部分,生成多张VPL的PSM
//		float vplPosBias = 0.05f;
//		vplCreation(selectedNum, vplDepthMapFBO, selectedSamplePointsPos, selectedSamplePointsNor, selectedSamplePointsCol, vplPosBias);
//		//最多支持32张贴图
//		selectedNum = min(selectedNum, maxSupportTex);
//
//#ifdef VPL_ILLUMINATION
//		//在之前的pMapUsageShader的基础上修改，希望渲染顶部光源以及若干vpl的阴影
//		//目前，顶部光源使用双面PSM，其他vpl使用单面PSM
//		//加入了顶部光源的cube map，通过在globalMapUsage.fs里选择使用PSM或CSM
//		glViewport(0, 0, width, height);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		//vpl的数量是selectedNum，int VPL_NUM = 1;
//		globalPSMUsageShader->use();
//		vplIllumination(selectedNum, selectedSamplePointsPos, selectedSamplePointsNor, selectedSamplePointsCol, vplPosBias);
//
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, depthMapFront);
//		//glActiveTexture(GL_TEXTURE1);
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, depthMapBack);
//		glActiveTexture(GL_TEXTURE2);
//		glBindTexture(GL_TEXTURE_CUBE_MAP, lightdepthCubeFBO.m_nTexture0Id);
//		//若干vpl的PSM
//		for (int i = 0; i < selectedNum; i++)
//		{
//			glActiveTexture(GL_TEXTURE3 + i);
//			glBindTexture(GL_TEXTURE_2D, vplDepthMap[i]);
//		}
//		
//		//ObjectManager::getInstance()->drawObject("cornellbox", globalPSMUsageShader->GetProgram());
//#endif // VPL_ILLUMINATION
//#pragma endregion
//		*/
//		
//		// draw sample point
//		glDisable(GL_DEPTH_TEST);
//		glViewport(0, 0, width, height);
//		GLuint pointshader = AssetManager::getInstance()->shaderPrograms["pointshader"].GetProgram();
//		glUseProgram(pointshader);
//		glBindVertexArray(vplVAO);
//		glUniformMatrix4fv(glGetUniformLocation(pointshader, "model"), 1, GL_FALSE, &(mat4(1.0f)[0][0]));
//		glUniformMatrix4fv(glGetUniformLocation(pointshader, "view"), 1, GL_FALSE, &(camera.getViewMatrix())[0][0]);
//		glUniformMatrix4fv(glGetUniformLocation(pointshader, "projection"), 1, GL_FALSE, &(camera.getProjectionMatrix())[0][0]);
//		glPointSize(5);
//		glLineWidth(2);
//		glDrawArrays(GL_POINTS, 0, selectedNum);
//		glBindVertexArray(0);
//		
//		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		//glDisable(GL_DEPTH_TEST);
//		//glClearColor(0, 0, 0, 1);
//		//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//		//screenSdr.use();
//		//glActiveTexture(GL_TEXTURE0);
//		//glBindTexture(GL_TEXTURE_2D, vplDepthMap[20]);
//		//drawQuad();
//
//
//		// per-frame time logic
//		// --------------------
//		float currentFrame = glfwGetTime();
//		nbframes++;
//		if (FPS_TEST && currentFrame - lasttime >= 1.0)
//		{
//			timetxt = "FPS: ";
//			timetxt += StringTool::int2string(nbframes);
//			nbframes = 0;
//			lasttime += 1.0;
//		}
//		textRender->RenderText(timetxt, 25.0f, 25.0f, 0.4f, glm::vec3(0.3, 0.7f, 0.9f));
//
//		if (saveImageFlag)
//		{
//			saveImageFlag = false;
//			ImageTool::SaveResizedImage("midpic.png", width, height, resizeRate);
//		}
//		//draw2D();
//
//		window->flush();
//		//break;
//	}
//
//	//ofstream fout("res.txt");
//	//fout.close();
//
//	delete[] samplePointsPos;
//	delete[] samplePointsNor;
//	delete[] samplePointsCol;
//	//cell-------------------------->
//	glDeleteVertexArrays(1, &vplVAO);
//	glDeleteBuffers(1, &vplVBO);
//	glDeleteTextures(1, &depthMapFront);
//	glDeleteTextures(1, &depthMapBack);
//	glDeleteTextures(maxSupportTex, vplDepthMap);
//	glDeleteFramebuffers(maxSupportTex, vplDepthMapFBO);
//
//	//DeleteFrameBuffer(tframe);
//	window->close();
//}
//
//
//void Scene::setupAnimation() {
//	twinsmap["cuberotate"] = new Twin<float>(10, 0, 90);
//	twinsmap["cubetranslate"] = new Twin<float>(10, 0, 5);
//}
//
//void Scene::animate() {
//	float cubetranslate = twinsmap["cubetranslate"]->update();
//	ObjectManager::getInstance()->objects["cube"].setPos(light.pos + vec3(cubetranslate, 0, 0));
//}
//
//// draw normal scene
//void Scene::draw(Shader norshader, Shader skyshader) {
//	norshader.use();
//	light.bindLight(norshader);
//	norshader.setVec3("viewPos", spcamera.getVirtulPos());
//	norshader.setMat4("view", camera.getViewMatrix());
//	norshader.setMat4("projection", camera.getProjectionMatrix());
//	norshader.setMat4("eyeMat", mat4(1.0f));
//
//	ObjectManager::getInstance()->drawObject("cornellbox", norshader.GetProgram());
//	ObjectManager::getInstance()->drawObject("cube", norshader.GetProgram());
//
//
//	/*skyshader.use();
//	skyshader.setVec3("viewPos", spcamera.getVirtulPos());
//	skyshader.setMat4("view", camera.getViewMatrix());
//	skyshader.setMat4("projection", camera.getProjectionMatrix());
//	skyshader.setMat4("eyeMat", mat4(1.0f));
//
//	ObjectManager::getInstance()->drawObject("skybox", skyshader.GetProgram());*/
//}
//
//void Scene::drawModelWithColor(char* modelId) {
//	GLuint color = AssetManager::getInstance()->shaderPrograms["colorshader"].GetProgram();;
//	glUseProgram(color);
//	glUniformMatrix4fv(glGetUniformLocation(color, "view"), 1, GL_FALSE, &(spcamera.getVirtualViewMatrix())[0][0]);
//	glUniformMatrix4fv(glGetUniformLocation(color, "projection"), 1, GL_FALSE, &(camera.getProjectionMatrix())[0][0]);
//	glUniformMatrix4fv(glGetUniformLocation(color, "eyeMat"), 1, GL_FALSE, &(mat4(1.0f)[0][0]));
//	ObjectManager::getInstance()->drawObject(modelId, color);
//}
//
//
//void Scene::vplIllumination(int selectedNum, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol, float vplPosBias) {
//	// world view and projection(for VS), world即之后的model
//	globalPSMUsageShader->setMat4("projection", camera.getProjectionMatrix());
//	globalPSMUsageShader->setMat4("view", camera.getViewMatrix());
//
//	globalPSMUsageShader->setInt("depthMapFront", 0);
//	globalPSMUsageShader->setInt("depthMapBack", 1);
//	globalPSMUsageShader->setInt("depthMap", 2);
//	// light view(for FS)
//	globalPSMUsageShader->setMat4("lightView", light.getViewMatrix());
//	// the light position in world coordinates
//	globalPSMUsageShader->setVec3("lightPos", light.pos);
//
//	globalPSMUsageShader->setVec3("lightNor", glm::zero<vec3>() - unitY);
//	globalPSMUsageShader->setVec3("lightCol", vec3(0.8f, 0.8f, 0.8f));
//	//pMapUsageShader.setVec3("lightCol", vec3(selectedSamplePointsCol[0], selectedSamplePointsCol[1], selectedSamplePointsCol[2]));
//	// the SHADOW_EPSILON, the necessary textures, light and material colors etc
//	globalPSMUsageShader->setVec3("viewPos", camera.pos);
//	globalPSMUsageShader->setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
//	globalPSMUsageShader->setFloat("near_plane", light_near_plane);
//	globalPSMUsageShader->setFloat("far_plane", light_far_plane);
//	globalPSMUsageShader->setInt("vplNUM", selectedNum);
//	//设置若干vpl的属性以及对应的PSM
//	//float vplConstant = 1.0f, vplLinear = 0.7f, vplCuadratic = 1.8f;
//
//	float vplConstant = 0.5f, vplLinear = 0.3f, vplCuadratic = 0.2f;
//	for (int i = 0; i < selectedNum; i++)
//	{
//		glm::vec3 vplNor(selectedSamplePointsNor[3 * i], selectedSamplePointsNor[3 * i + 1], selectedSamplePointsNor[3 * i + 2]);
//		glm::vec3 vplPos(selectedSamplePointsPos[3 * i], selectedSamplePointsPos[3 * i + 1], selectedSamplePointsPos[3 * i + 2]);
//		glm::vec3 vplCol(selectedSamplePointsCol[3 * i], selectedSamplePointsCol[3 * i + 1], selectedSamplePointsCol[3 * i + 2]);
//
//		//关于怎么计算凝视点，后面就想办法看向盒子中心？
//		glm::mat4 vplView = glm::lookAt(vplPos, vplPos + vplNor, glm::zero<vec3>() + unitY);
//		//为了消除光斑
//		vplPos -= (5 * vplPosBias * vplNor);
//		globalPSMUsageShader->setVec3("pointLights[" + to_string(i) + "].position", vplPos);
//		globalPSMUsageShader->setVec3("pointLights[" + to_string(i) + "].normal", vplNor);
//		globalPSMUsageShader->setMat4("pointLights[" + to_string(i) + "].view", vplView);
//		globalPSMUsageShader->setFloat("pointLights[" + to_string(i) + "].constant", vplConstant);
//		globalPSMUsageShader->setFloat("pointLights[" + to_string(i) + "].linear", vplLinear);
//		globalPSMUsageShader->setFloat("pointLights[" + to_string(i) + "].quadratic", vplCuadratic);
//		globalPSMUsageShader->setVec3("pointLights[" + to_string(i) + "].diffuse", vplCol);
//		globalPSMUsageShader->setInt("vplDepthMap[" + to_string(i) + "]", i + 3);
//
//	}
//}