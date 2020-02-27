#include "scene.h"

void Scene::vplCreation(int selectedNum, GLuint* vplDepthMapFBO, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol, float vplPosBias) {
	//法线的计算应该可以在shader里用normalize计算
	//for (int i = 0; i < selectedNum; i++)
	//{
	//	float s = sqrt(pow(selectedSamplePointsNor[3 * i], 2) + pow(selectedSamplePointsNor[3 * i + 1], 2) + pow(selectedSamplePointsNor[3 * i + 2], 2));
	//	selectedSamplePointsNor[3 * i] /= s;
	//	selectedSamplePointsNor[3 * i + 1] /= s;
	//	selectedSamplePointsNor[3 * i + 2] /= s;
	//	//vpl的位置，法线以及颜色	
	//	glm::vec3 vplNor(selectedSamplePointsNor[3 * i], selectedSamplePointsNor[3 * i + 1], selectedSamplePointsNor[3 * i + 2]);
	//	glm::vec3 vplCol(selectedSamplePointsCol[3 * i], selectedSamplePointsCol[3 * i + 1], selectedSamplePointsCol[3 * i + 2]);
	//	//glm::vec3 vplPos(selectedSamplePointsPos[3 * i] + vplPosBias, selectedSamplePointsPos[3 * i + 1] + vplPosBias, selectedSamplePointsPos[3 * i + 2] + vplPosBias);
	//	glm::vec3 vplPos(selectedSamplePointsPos[3 * i], selectedSamplePointsPos[3 * i + 1], selectedSamplePointsPos[3 * i + 2]);
	//	//设置偏移，保证不与面片冲突
	//	vplPos += (vplPosBias * vplNor);
	//	//关于怎么计算凝视点，现在先知道右面墙，所以手动设置，后面就想办法看向盒子中心？]
	//	glm::mat4 vplView;
	//	if (i < 30)
	//	{
	//		vplView = glm::lookAt(vplPos, vplPos + vplNor, glm::zero<vec3>() + unitY);
	//	}
	//	else
	//	{
	//		vplPos -= (vplPosBias * vplNor);
	//		vec3 vpltoLight = vplPos - light.Position();
	//		float A = vplNor.x;
	//		float B = vplNor.y;
	//		float C = vplNor.z;
	//		float D = -(vplNor.x * vplPos.x) - (vplNor.y * vplPos.y) - (vplNor.z * vplPos.z);
	//		float t = (A * light.Position().x + B * light.Position().y + C * light.Position().z + D) / (pow(A, 2) + pow(B, 2) + pow(C, 2));
	//		float x = light.Position().x - (A * t);
	//		float y = light.Position().y - (B * t);
	//		float z = light.Position().z - (C * t);
	//		vec3 pjPoint = vec3(x, y, z);
	//		vec3 up = normalize(pjPoint - light.Position());
	//		vplView = glm::lookAt(vplPos, vplPos + vplNor, up);
	//	}
	//	vplPSMCreationShader->use();
	//	vplPSMCreationShader->setMat4("vplView", vplView);
	//	vplPSMCreationShader->setFloat("near_plane", light_near_plane);
	//	vplPSMCreationShader->setFloat("far_plane", light_far_plane);
	//	vplPSMCreationShader->setFloat("direction", -1);
	//	glViewport(0, 0, VPL_SHADOW_WIDTH, VPL_SHADOW_HEIGHT);
	//	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFrontFBO);
	//	glBindFramebuffer(GL_FRAMEBUFFER, vplDepthMapFBO[i]);
	//	glClear(GL_DEPTH_BUFFER_BIT);
	//	//drawObject里设置了model矩阵
	//	ObjectManager::getInstance()->drawObject("cornellbox", vplPSMCreationShader->GetProgram());
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//}

	////以下是使用部分
	//glViewport(0, 0, width, height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	////光源衰减设置
	//float constant = 0.5f, linear = 0.3f, quadratic = 0.2f;
	//vplPSMUsageShader.use();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, depthMapFront);
	//vplPSMUsageShader.setInt("depthMapFront", 0);
	//// world view and projection(for VS), world即之后的model		
	//vplPSMUsageShader.setMat4("vplView", vplView);
	//vplPSMUsageShader.setVec3("vplPos", vplPos);
	//vplPSMUsageShader.setVec3("vplCol", vplCol);
	//vplPSMUsageShader.setMat4("projection", camera.getProjectionMatrix());
	//vplPSMUsageShader.setMat4("view", camera.getViewMatrix());
	//vplPSMUsageShader.setVec3("viewPos", camera.pos);
	//vplPSMUsageShader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
	//vplPSMUsageShader.setFloat("near_plane", near_plane);
	//vplPSMUsageShader.setFloat("far_plane", far_plane);
	//vplPSMUsageShader.setVec3("pointLights[0].position", vplPos);
	//vplPSMUsageShader.setFloat("pointLights[0].constant", constant);
	//vplPSMUsageShader.setFloat("pointLights[0].linear", linear);
	//vplPSMUsageShader.setFloat("pointLights[0].quadratic", quadratic);
	//vplPSMUsageShader.setVec3("pointLights[0].diffuse", vplCol);
	//ObjectManager::getInstance()->drawObject("cornellbox", pMapUsageShader.GetProgram());
}

void Scene::setupDepthFBO(GLuint &depthMapFront, GLuint &depthMapBack, GLuint depthMapFrontFBO, GLuint depthMapBackFBO,
	int selectedNum, GLuint* vplDepthMap, GLuint* vplDepthMapFBO)
{
	glGenTextures(1, &depthMapFront);
	glBindTexture(GL_TEXTURE_2D, depthMapFront);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFrontFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapFront, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create depth back texture
	glGenTextures(1, &depthMapBack);
	glBindTexture(GL_TEXTURE_2D, depthMapBack);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapBackFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapBack, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// create depth vpl texture
	for (int i = 0; i < selectedNum; i++)
	{
		glGenTextures(1, &vplDepthMap[i]);
		glBindTexture(GL_TEXTURE_2D, vplDepthMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, VPL_SHADOW_WIDTH, VPL_SHADOW_WIDTH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, vplDepthMapFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, vplDepthMap[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

vec3 eachSamplePointTransPos[10000];
// calculate importance for each light, by compute all points boundingbox aera in each light view
double  Scene::calcuLightImportance(vec3 lightpos, vec3 lightdir, int pointnum, float* gpos) {
	Camera cam;
	cam.ClipPlaneNear(1.0f);
	cam.ClipPlaneFar(50.0f);
	cam.FieldOfView(150);
	cam.Position(lightpos);
	mat4 proj = cam.ProjectionMatrix();
	mat4 view = glm::lookAt(lightpos, lightpos + lightdir, vec3(0, 1, 0));
	for (int i = 0; i < pointnum; ++i) {
		if (gpos[3 * i] < -10000) {
			continue;
		}
		vec4 transpos = proj * view * vec4(gpos[3 * i], gpos[3 * i + 1], gpos[3 * i + 2], 1);
		eachSamplePointTransPos[i] = vec3(transpos.x, transpos.y, transpos.z);
	}
	float xlowbound = 1;
	float xupbound = -1;
	float ylowbound = 1;
	float yupbound = -1;
	for (int i = 0; i < pointnum; ++i) {
		if (eachSamplePointTransPos[i].z <= 0) {
			continue;
		}
		xlowbound = std::min(xlowbound, eachSamplePointTransPos[i].x);
		ylowbound = std::min(ylowbound, eachSamplePointTransPos[i].y);
		xupbound = std::max(xupbound, eachSamplePointTransPos[i].x);
		yupbound = std::max(yupbound, eachSamplePointTransPos[i].y);
	}
	xlowbound = glm::clamp(xlowbound, -1.0f, 1.0f);
	ylowbound = glm::clamp(ylowbound, -1.0f, 1.0f);
	xupbound = glm::clamp(xupbound, -1.0f, 1.0f);
	yupbound = glm::clamp(yupbound, -1.0f, 1.0f);
	if (xlowbound > xupbound || ylowbound > yupbound) {
		return 0;
	}

	return (xupbound - xlowbound)*(yupbound - ylowbound) * 0.25f;
}

// simplified mode: fetch point from model
int Scene::getSamplePointAndNor(float* &pos, float* &nor) {
	int num = 0;
	auto mesh = ObjectManager::getInstance()->objects["samplecornell"].meshIndex;
	vector<ModelClass::GLMesh*> &meshes = AssetManager::getInstance()->meshes;
	mat4 modelmat = ObjectManager::getInstance()->objects["cornellbox"].getModelMat();
	mat3 normat = transpose(inverse(modelmat));
	num = meshes[mesh.first]->vertices.size();
	num /= 5;
	pos = new float[num * 3];
	nor = new float[num * 3];

	for (int i = 0; i < num; ++i) {
		ModelClass::Vertex ver = meshes[mesh.first]->vertices[i * 5];
		//meshes[mesh.first]->meterial->diffuseColor;
		//ver.TexCoords; 
		//glGetTexImage();
		vec4 tpos = modelmat * vec4(ver.Position.x, ver.Position.y, ver.Position.z, 1);
		vec3 tnor = normat * vec3(ver.Normal.x, ver.Normal.y, ver.Normal.z);
		pos[3 * i] = tpos.x;
		pos[3 * i + 1] = tpos.y;
		pos[3 * i + 2] = tpos.z;
		nor[3 * i] = tnor.x;
		nor[3 * i + 1] = tnor.y;
		nor[3 * i + 2] = tnor.z;
	}
	return num;
}

int Scene::getSamplePointAndNor(float* &pos, float* &nor, float* &col) {
	int num = 0;
	auto mesh = ObjectManager::getInstance()->objects["samplecornell"].meshIndex;
	vector<ModelClass::GLMesh*> &meshes = AssetManager::getInstance()->meshes;
	mat4 modelmat = ObjectManager::getInstance()->objects["cornellbox"].getModelMat();
	mat3 normat = transpose(inverse(modelmat));
	num = meshes[mesh.first]->vertices.size();
	num /= 5;
	pos = new float[num * 3];
	nor = new float[num * 3];
	col = new float[num * 3];

	for (int i = 0; i < num; ++i) {
		ModelClass::Vertex ver = meshes[mesh.first]->vertices[i * 5];

		//如何确定该点所在的mesh，meshes[7]和meshes[8]的diffuse分别是绿色和红色
		//vec3 tcol = meshes[mesh.first]->meterial->diffuseColor;
		vec3 tcol = meshes[7]->meterial->diffuseColor;
		//ver.TexCoords; 
		//glGetTexImage();

		vec4 tpos = modelmat * vec4(ver.Position.x, ver.Position.y, ver.Position.z, 1);
		vec3 tnor = normat * vec3(ver.Normal.x, ver.Normal.y, ver.Normal.z);
		pos[3 * i] = tpos.x;
		pos[3 * i + 1] = tpos.y;
		pos[3 * i + 2] = tpos.z;
		nor[3 * i] = tnor.x;
		nor[3 * i + 1] = tnor.y;
		nor[3 * i + 2] = tnor.z;
		col[3 * i] = tcol.x;
		col[3 * i + 1] = tcol.y;
		col[3 * i + 2] = tcol.z;
		//cout << "tcol.x: " << tcol.x << endl;
		//cout << "tcol.y: " << tcol.y << endl;
		//cout << "tcol.z: " << tcol.z << endl;
	}
	return num;
}
