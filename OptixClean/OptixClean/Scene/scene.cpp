#include "../optixPart/optixSampler.h"
#include "scene.h"
#include "sampleAssistance.h"
#include "../Renderer/CubeRender.h"
#include "../Common/StringTool.h"

//#define GBUFFER
#define VPL_ILLUMINATION
//输出FPS信息
bool FPS_TEST = 1;

bool saveImageFlag;
bool shadows = true;
float resizeRate = 1;

void Scene::initLight() {
	light.resize(10);
	light[0].Position(vec3(0, 2, -4));
	light[0].Forward(glm::zero<vec3>() - unitY);
}

void Scene::initCamera() {
	// farClip默认500
	camera.FieldOfView(glm::radians(60.f));
	camera.AspectRatio(1);
	camera.ClipPlaneNear(0.3f);
	camera.Position(vec3(0.4, 1, -10.5));
	//camera.Forward(ObjectManager::getInstance()->objects["cornellbox"].Position() - camera.Position());
}

void Scene::initSceneObject() {
	quad.Load();

	ObjectManager::getInstance()->objects["floor"].Scale(ObjectManager::getInstance()->objects["floor"].Scale() 
		* vec3(2, 2, 2));
	ObjectManager::getInstance()->objects["cube"].Scale(ObjectManager::getInstance()->objects["cube"].Scale()
		* vec3(0.01, 0.01, 0.01));
	ObjectManager::getInstance()->objects["cube"].Position(light[0].Position());
	
	GLTransform &trans = ObjectManager::getInstance()->objects["cornellbox"].transform;
	trans.Scale(trans.Scale() * vec3(0.1, 0.1, 0.1));
	//trans.Position(vec3(0, 1, 7));
	trans.Rotation(vec3(-glm::pi<float>(), glm::pi<float>() / 2, 0));
}

void Scene::makeBoundaries() {
	float bound = 10;
	boundaries.MinPoint(vec3(-bound, -bound, -bound));
	boundaries.MaxPoint(vec3(bound, bound, bound));
}

void Scene::init() {

	Loader loader;
	loader.load("config.txt");

	width = 900;
	height = 900;
	window = WindowCallBack::MyWindow::InitWindow(width, height, "driver");
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "%s\n", glewGetErrorString(err));
		return;
	}

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxSupportTex);
	AssetManager *am = AssetManager::getInstance();
	ObjectManager *om = ObjectManager::getInstance();

	am->readConfig(loader.stringMap["model"] + loader.stringMap["shader"] + "end\n");
	om->readConfig(loader.stringMap["model"] + "end\n");
	shader = &(am->shaderPrograms["roll"]);
	shaderBox = &(am->shaderPrograms["box"]);

	initLight();
	initCamera();
	initSceneObject();
	makeBoundaries();
	
	Shader screenSdr = AssetManager::getInstance()->shaderPrograms["screenshader"];
	screenSdr.use();
	screenSdr.setInt("screenTexture", 0);
	glUseProgram(0);
	
	// 写入gbuffer
	geometryPassShader = &AssetManager::getInstance()->shaderPrograms["geometryPassShader"];

	CreateFrameBuffer(width, height, tframe);
	CreateGBuffer(width, height, gbufferFBO);
	CreateCubemapFramebuffer(SHADOW_WIDTH, SHADOW_HEIGHT, lightdepthCubeFBO);

	frameNum = 0;

	setupAnimation();
	textRender = new TextRender(width, height);
	voxelRenderer = new VoxelRender();
	shadowRenderer = new ShadowMapRenderer();
}

void Scene::run() {

	glEnable(GL_DEPTH_TEST);
	Shader screenSdr = AssetManager::getInstance()->shaderPrograms["screenshader"];

	int nbframes = 0;
	std::srand(0);
	double lasttime = glfwGetTime();
	string timetxt("FPS:");

	//FPS_TEST = 0;
	while (!glfwWindowShouldClose(window->window))
	{
		// move light position over time
		//light.pos.y = sin(glfwGetTime() * 0.5) * 3.0 + 2;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 0);
		glEnable(GL_DEPTH_TEST);
		if (!handleInput())
			break;
		animate();
		
		//bindMatrices(*shader, camera);
		//draw(*shader, *shader);

		//shadowRenderer->Render();
		voxelRenderer->Render();


		// clear all transform change info
		GLTransform::CleanEventMap();
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		nbframes++;
		if (FPS_TEST && currentFrame - lasttime >= 1.0)
		{
			timetxt = "FPS: ";
			timetxt += StringTool::int2string(nbframes);
			nbframes = 0;
			lasttime += 1.0;
		}
		textRender->RenderText(timetxt, 25.0f, 25.0f, 0.4f, glm::vec3(0.3, 0.7f, 0.9f));

		if (saveImageFlag)
		{
			saveImageFlag = false;
			ImageTool::SaveResizedImage("midpic.png", width, height, resizeRate);
		}
		//draw2D();

		window->flush();
		//break;
	}

	//ofstream fout("res.txt");
	//fout.close();

	//DeleteFrameBuffer(tframe);
	window->close();
}


void Scene::setupAnimation() {
	twinsmap["cuberotate"] = new Twin<float>(10, 0, 90);
	twinsmap["cubetranslate"] = new Twin<float>(10, 0, 5);
}

void Scene::animate() {
	float cubetranslate = twinsmap["cubetranslate"]->update();
	ObjectManager::getInstance()->objects["cube"].Position(light[0].Position());
}

void Scene::bindMatrices(Shader& norshader, Camera& cam) {
	norshader.setVec3("viewPos", cam.Position());
	norshader.setMat4("view", cam.ViewMatrix());
	norshader.setMat4("projection", cam.ProjectionMatrix());
	norshader.setMat4("eyeMat", mat4(1.0f));
}

// draw normal scene
void Scene::draw(Shader& shader) {
	shader.use();
	light[0].bindLight(shader);
	ObjectManager::getInstance()->drawObject("cornellbox", shader.GetProgram());
	ObjectManager::getInstance()->drawObject("cube", shader.GetProgram());
}

void Scene::drawSkybox(Shader& skyshader) {
	skyshader.use();
	ObjectManager::getInstance()->drawObject("skybox", skyshader.GetProgram());
}

void Scene::drawModelWithColor(char* modelId) {
	GLuint color = AssetManager::getInstance()->shaderPrograms["colorshader"].GetProgram();;
	glUseProgram(color);
	glUniformMatrix4fv(glGetUniformLocation(color, "view"), 1, GL_FALSE, &(spcamera.getVirtualViewMatrix())[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(color, "projection"), 1, GL_FALSE, &(camera.ProjectionMatrix())[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(color, "eyeMat"), 1, GL_FALSE, &(mat4(1.0f)[0][0]));
	ObjectManager::getInstance()->drawObject(modelId, color);
}


void Scene::vplIllumination(int selectedNum, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol, float vplPosBias) {
	// world view and projection(for VS), world即之后的model
	globalPSMUsageShader->setMat4("projection", camera.ProjectionMatrix());
	globalPSMUsageShader->setMat4("view", camera.ViewMatrix());

	globalPSMUsageShader->setInt("depthMapFront", 0);
	globalPSMUsageShader->setInt("depthMapBack", 1);
	globalPSMUsageShader->setInt("depthMap", 2);
	// light view(for FS)
	globalPSMUsageShader->setMat4("lightView", lookAt(light[0].Position(), light[0].Position()+light[0].Forward(), light[0].Up()));
	// the light position in world coordinates
	globalPSMUsageShader->setVec3("lightPos", light[0].Position());

	globalPSMUsageShader->setVec3("lightNor", glm::zero<vec3>() - unitY);
	globalPSMUsageShader->setVec3("lightCol", vec3(0.8f, 0.8f, 0.8f));
	//pMapUsageShader.setVec3("lightCol", vec3(selectedSamplePointsCol[0], selectedSamplePointsCol[1], selectedSamplePointsCol[2]));
	// the SHADOW_EPSILON, the necessary textures, light and material colors etc
	globalPSMUsageShader->setVec3("viewPos", camera.Position());
	globalPSMUsageShader->setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
	globalPSMUsageShader->setFloat("near_plane", light_near_plane);
	globalPSMUsageShader->setFloat("far_plane", light_far_plane);
	globalPSMUsageShader->setInt("vplNUM", selectedNum);
	//设置若干vpl的属性以及对应的PSM
	//float vplConstant = 1.0f, vplLinear = 0.7f, vplCuadratic = 1.8f;

	float vplConstant = 0.5f, vplLinear = 0.3f, vplCuadratic = 0.2f;
	for (int i = 0; i < selectedNum; i++)
	{
		glm::vec3 vplNor(selectedSamplePointsNor[3 * i], selectedSamplePointsNor[3 * i + 1], selectedSamplePointsNor[3 * i + 2]);
		glm::vec3 vplPos(selectedSamplePointsPos[3 * i], selectedSamplePointsPos[3 * i + 1], selectedSamplePointsPos[3 * i + 2]);
		glm::vec3 vplCol(selectedSamplePointsCol[3 * i], selectedSamplePointsCol[3 * i + 1], selectedSamplePointsCol[3 * i + 2]);

		//关于怎么计算凝视点，后面就想办法看向盒子中心？
		glm::mat4 vplView = glm::lookAt(vplPos, vplPos + vplNor, glm::zero<vec3>() + unitY);
		//为了消除光斑
		vplPos -= (5 * vplPosBias * vplNor);
		globalPSMUsageShader->setVec3("pointLights[" + to_string(i) + "].position", vplPos);
		globalPSMUsageShader->setVec3("pointLights[" + to_string(i) + "].normal", vplNor);
		globalPSMUsageShader->setMat4("pointLights[" + to_string(i) + "].view", vplView);
		globalPSMUsageShader->setFloat("pointLights[" + to_string(i) + "].constant", vplConstant);
		globalPSMUsageShader->setFloat("pointLights[" + to_string(i) + "].linear", vplLinear);
		globalPSMUsageShader->setFloat("pointLights[" + to_string(i) + "].quadratic", vplCuadratic);
		globalPSMUsageShader->setVec3("pointLights[" + to_string(i) + "].diffuse", vplCol);
		globalPSMUsageShader->setInt("vplDepthMap[" + to_string(i) + "]", i + 3);

	}
}