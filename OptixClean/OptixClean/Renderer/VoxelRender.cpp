#include "VoxelRender.h"
#include "../Scene/scene.h"
#include "../Renderer/GBufferRender.h"

vec4 convRGBA8ToVec4(uint val)
{
	return vec4(float((val & 0x000000FF)), float((val & 0x0000FF00) >> 8U),
		float((val & 0x00FF0000) >> 16U), float((val & 0xFF000000) >> 24U));
}
vec4 convRGB10_2AToVec4(uint val)
{
	return vec4(float((val & (0x000003FF))), float((val & (0x000FFD00)) >> 10U),
		float((val & 0x3FF00000) >> 20U), 0);
}
void VoxelRender::testImageStore()
{
	auto &forStoreShader = AssetManager::getInstance()->shaderPrograms["forTestStore"];
	auto &forLoadShader = AssetManager::getInstance()->shaderPrograms["forTestLoad"];
	//Texture3D forCompare = Texture3D(sizeof(GLuint) * 256, GL_R, GL_R32UI);
	AtomicBuffer ato= AtomicBuffer(1);
	TextureBuffer forTest = TextureBuffer(sizeof(GLuint) * 1024, GL_R32UI);
	forStoreShader.use();
	glBindImageTexture(0, forTest.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ato.bufId);
	int computeGroupDim = 1024 / 64;
	glDispatchCompute(computeGroupDim, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

	forLoadShader.use();
	glBindImageTexture(0, forTest.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ato.bufId);
	glDispatchCompute(computeGroupDim, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
	int tileAllocated = ato.GetCounter();
	cout << "for Test :"<<tileAllocated << endl;
	ato.ResetCounter(0);
}
void VoxelRender::Render() {
	if (!initFlag) {
		//
		
		SetupGlobalUniform(Scene::getInstance()->boundaries);
		VoxelizeScene(0);
		GLuint count = atomicCounter.GetCounter();
		numVoxel = count;
		atomicCounter.ResetCounter(0);
		if (true)
		{
			initFlag = true;
			voxelPos = TextureBuffer(sizeof(GLuint) * numVoxel, GL_R32UI);
			voxelAlbedo = TextureBuffer(sizeof(GLuint) * numVoxel, GL_R32UI);
			voxelNormal = TextureBuffer(sizeof(GLuint) * numVoxel, GL_R32UI);
			voxelEmissive = TextureBuffer(sizeof(GLuint) * numVoxel, GL_R32UI);
			//Calculate the maximum possilbe node number
			numOfNode = 1;
			int tmpNum = 1;
			levelOctree = 0;
			for (int i = 1; tmpNum < voxelCount; i++)
			{
				tmpNum *= 8;
				levelOctree++;
				numOfNode += tmpNum;
			}
			//cout << "The Level Octree: " << levelOctree << endl;
			//cout << " need number of OCtreeNode:" << numOfNode << endl;
			octreeNode = TextureBuffer(sizeof(GLuint) * numOfNode, GL_R32UI);
			octreeNodeAlbedo = TextureBuffer(sizeof(GLuint) * numOfNode, GL_R32UI);
		}
		VoxelizeScene(1);
		BuildSVO();
		
		SVOTo3Dtex();
		
		
		//testImageStore();
	}
	//InjectRadiance();
	DrawVoxels();
}

void VoxelRender::SetupGlobalUniform(BoundingBox sceneBox) {
	auto axisSize = sceneBox.Extent() * 2.0f;
	auto &center = sceneBox.Center();
	volumeGridSize = glm::max(axisSize.x, glm::max(axisSize.y, axisSize.z));
	voxelSize = volumeGridSize / volumeDimension;
	auto halfSize = volumeGridSize / 2.0f;
	// projection matrices
	auto projection = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, 0.0f,
		volumeGridSize);
	// view matrices
	viewProjectionMatrix[0] = lookAt(center + glm::vec3(halfSize, 0.0f, 0.0f),
		center, glm::vec3(0.0f, 1.0f, 0.0f));
	viewProjectionMatrix[1] = lookAt(center + glm::vec3(0.0f, halfSize, 0.0f),
		center, glm::vec3(0.0f, 0.0f, -1.0f));
	viewProjectionMatrix[2] = lookAt(center + glm::vec3(0.0f, 0.0f, halfSize),
		center, glm::vec3(0.0f, 1.0f, 0.0f));
	int i = 0;

	for (auto &matrix : viewProjectionMatrix)
	{
		matrix = projection * matrix;
		viewProjectionMatrixI[i++] = inverse(matrix);
	}
}

void VoxelRender::VoxelizeScene(int isStore) {
	static float zero[] = { 0, 0, 0, 0 };
	static float sZero = 0.0f;

	Scene *scene = Scene::getInstance();	//**如果对于不同物体可以更改
	if (!scene) { return; }

	auto &prog = AssetManager::getInstance()->shaderPrograms["voxelization"];
	auto &sceneBox = scene->boundaries;
	static auto &camera = scene->camera;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// clear and setup viewport
	glColorMask(false, false, false, false);
	glViewport(0, 0, volumeDimension, volumeDimension);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// active voxelization pass program
	prog.use();
	// rendering flags
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	camera.DoFrustumCulling(false);
	// pass voxelization uniforms
	prog.setMat4("viewProjections[0]", viewProjectionMatrix[0]);
	prog.setMat4("viewProjections[1]", viewProjectionMatrix[1]);
	prog.setMat4("viewProjections[2]", viewProjectionMatrix[2]);

	prog.setMat4("viewProjectionsI[0]", viewProjectionMatrixI[0]);
	prog.setMat4("viewProjectionsI[1]", viewProjectionMatrixI[1]);
	prog.setMat4("viewProjectionsI[2]", viewProjectionMatrixI[2]);

	prog.setUInt("volumeDimension", volumeDimension);
	prog.setVec3("worldMinPoint", sceneBox.MinPoint());
	prog.setFloat("voxelScale", 1.0f / volumeGridSize);
	prog.setInt("isStore", isStore);
	if (isStore == 1)
	{
		// bind images	
		glBindImageTexture(0, voxelPos.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGB10_A2UI);
		glBindImageTexture(1, voxelAlbedo.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		glBindImageTexture(2, voxelNormal.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		glBindImageTexture(3, voxelEmissive.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	}
	atomicCounter.ResetCounter(0);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter.bufId);
	// draw scene triangles
	scene->draw(prog);
	// sync barrier
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	//if (true&&isStore == 1 )
	//{
	//	//Get the buffer data
	//	glBindBuffer(GL_TEXTURE_BUFFER, voxelAlbedo.tbo);
	//	unsigned int *output_image = new unsigned int[numVoxel];
	//	glGetBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(GLuint)*numVoxel, output_image);
	//	int count = 0;
	//	for (int i = 0; i < numVoxel; i++)
	//	{
	//		vec4 result = convRGBA8ToVec4(output_image[i]);
	//		count++;
	//	}
	//	//cout << "albedo: " << count << endl;
	//	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	//}
}

void VoxelRender::DrawVoxels()
{
	static auto scene = Scene::getInstance();
	auto camera = scene->camera;

	if (!scene ) { return; }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// clear and setup viewport
	glColorMask(true, true, true, true);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glViewport(0, 0, scene->width, scene->height);

	// Open GL flags
	glClearDepth(1.0f);

	// rendering flags
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	auto &prog = AssetManager::getInstance()->shaderPrograms["voxelDrawer"];
	prog.use();
	// voxel grid projection matrices
	auto &sceneBox = scene->boundaries;
	auto &viewProjection = camera.ViewProjectionMatrix();
	auto vDimension = static_cast<unsigned>(volumeDimension / pow(2.0f,
		drawMipLevel));
	auto vSize = volumeGridSize / vDimension;
	// pass voxel drawer uniforms
	if (drawDirection == 7)
	{
		//glBindImageTexture(0, voxelNormal.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	}
	else if (drawDirection == 8)
	{
		//glBindImageTexture(0, voxelRadiance.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	}
	else if (drawMipLevel == 0)
	{
		glBindImageTexture(0, octree3D.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	}
	else if (drawMipLevel > 0)
	{
		//glBindImageTexture(0, voxelTexMipmap[drawDirection].id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	}
	
	auto model = scale(translate(mat4(1.0), sceneBox.MinPoint()), glm::vec3(vSize));
	prog.setUInt("volumeDimension", vDimension);
	prog.setMat4("matrices.modelViewProjection", viewProjection * model);
	prog.setVec4("colorChannels", drawColorChannels);
	auto &planes = camera.Frustum().Planes();

	for (auto i = 0; i < 6; i++)
	{
		prog.setVec4("frustumPlanes[" + std::to_string(i) + "]", planes[i]);
	}

	prog.setFloat("voxelSize", voxelSize);
	prog.setVec3("worldMinPoint", sceneBox.MinPoint());
	// bind vertex buffer array to draw, needed but all geometry is generated
	// in the geometry shader
	if (!voxelDrawerArray) {
		glGenVertexArrays(0, &voxelDrawerArray);
	}
	glBindVertexArray(voxelDrawerArray);
	glDrawArrays(GL_POINTS, 0, voxelCount);
}

void VoxelRender::InjectRadiance()
{

}

VoxelRender::VoxelRender()
{
	injectFirstBounce = true;
	drawMipLevel = drawDirection = 0;
	framestep = -1; // on need
	traceShadowCones = true;
	traceShadowHit = 0.5f;
	drawColorChannels = glm::vec4(1.0f);
	normalWeightedLambert = true;
	voxelDrawerArray = 0;
	SetupVoxelVolumes(256);

}

void VoxelRender::SetupVoxelVolumes(const unsigned int &dimension) {
	SetupGlobalUniform(Scene::getInstance()->boundaries);

	volumeDimension = dimension;
	voxelCount = volumeDimension * volumeDimension * volumeDimension;
	voxelSize = volumeGridSize / volumeDimension;
	octree3D = Texture3D(dimension, GL_RGBA8, GL_RGBA);
	atomicCounter = AtomicBuffer(1);

	for (int i = 0; i < 6; i++)
	{
		voxelTexMipmap[i] = Texture3D(dimension / 2, GL_RGBA8, GL_RGBA);
	}
}

void VoxelRender::BuildSVO()
{
	GLenum err;
	

	auto &nodeFlagShader = AssetManager::getInstance()->shaderPrograms["nodeFlag"];
	auto &nodeAllocShader = AssetManager::getInstance()->shaderPrograms["nodeAlloc"];
	auto &nodeInitShader = AssetManager::getInstance()->shaderPrograms["nodeInit"];
	auto &leafStoreShader = AssetManager::getInstance()->shaderPrograms["leafStore"];
	
	int nodeOffset = 0;
	int allocOffset = 1;
	vector<unsigned int> allocList; //the vector records the number of nodes in each tree level
	allocList.push_back(1); //root level has one node
	GLuint tileAllocated;	//how many node of allocation
	int dataWidth = 1024;
	int dataHeight = (numVoxel + 1023) / dataWidth;	//numVoxelFrag 记录Voxel的个数
	int groupDimX = dataWidth / 8;
	int groupDimY = (dataHeight + 7) / 8;

	float beforTime = glfwGetTime();
	for (int i = 0; i < levelOctree; i++)
	{
		//cout << "level i:" << i << endl;
		//node flag
		//atomicCounter.ResetCounter(0);
		nodeFlagShader.use();
		nodeFlagShader.setInt("u_level", i);
		nodeFlagShader.setUInt("u_voxelDim", volumeDimension);
		nodeFlagShader.setInt("u_numVoxel", numVoxel);
		glBindImageTexture(0, voxelPos.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGB10_A2UI);
		glBindImageTexture(1, octreeNode.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
		//glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter.bufId);
		//int computeGroupDim = (numVoxel + 63) / 64;
		glDispatchCompute(groupDimX, groupDimY, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//Get the number of node tiles to allocate in the next level
		//tileAllocated = atomicCounter.GetCounter();
		//cout << " flag : "<<tileAllocated << endl;
		//atomicCounter.ResetCounter(0);
		
		//node tile allocation
		nodeAllocShader.use();
		int numThread = allocList[i];
		nodeAllocShader.setInt("u_start", nodeOffset);
		nodeAllocShader.setInt("u_allocStart", allocOffset);
		nodeAllocShader.setInt("u_num", numThread);
		atomicCounter.ResetCounter(0);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter.bufId);
		glBindImageTexture(0, octreeNode.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
		int allocGroupDim = (allocList[i] + 63) / 64;
		glDispatchCompute(allocGroupDim, 1, 1);
		if (i == 7) 
			int a = 0;
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
		//Get the number of node tiles to allocate in the next level
		tileAllocated = atomicCounter.GetCounter();
		//cout << " alloc number: "<<tileAllocated << endl;
		//atomicCounter.ResetCounter(0);
		////Get the buffer data
		//glBindBuffer(GL_TEXTURE_BUFFER, octreeNode.tbo);
		//unsigned int *output_image = new unsigned int[numVoxel];
		//glGetBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(GLuint)*numVoxel, output_image);
		////if ((output_image[0] & 0x80000000) != 0)
		//cout << " flag have data" << (output_image[0] & 0x7FFFFFFF) << endl;
		//glBindBuffer(GL_TEXTURE_BUFFER, 0);

		//node tile initialization
		nodeInitShader.use();
		int nodeAllocated = tileAllocated * 8; //the number of need to be initialize
		nodeInitShader.setInt("u_allocStart", allocOffset);
		nodeInitShader.setInt("u_num", nodeAllocated);
		glBindImageTexture(0, octreeNode.texId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);
		int dataWidth = 1024;	//for parallel efficiency
		int dataHeight = (nodeAllocated + 1023) / dataWidth;
		int initGroupDimX = dataWidth / 8;
		int initGroupDimY = (dataHeight + 7) / 8;
		/*glDispatchCompute(initGroupDimX, initGroupDimY, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);*/
		//update offsets for next level
		allocList.push_back(nodeAllocated); //titleAllocated * 8 is the number of threads
											//we want to launch in the next level
		nodeOffset += allocList[i]; //nodeOffset is the starting node in the next level
		allocOffset += nodeAllocated; //allocOffset is the starting address of remaining free space
	}

	//node nonempty leaf flag
	nodeFlagShader.use();
	nodeFlagShader.setInt("u_level", levelOctree);
	nodeFlagShader.setUInt("u_voxelDim", volumeDimension);
	nodeFlagShader.setInt("u_numVoxel", numVoxel);
	glBindImageTexture(0, voxelPos.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGB10_A2UI);
	glBindImageTexture(1, octreeNode.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	//atomicCounter.ResetCounter(0);
	//glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter.bufId);
	//int computeGroupDim = (numVoxel + 63) / 64;
	glDispatchCompute(groupDimX, groupDimY, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//Get the number of node tiles to allocate in the next level
	//tileAllocated = atomicCounter.GetCounter();
	//cout << "last flag : " << tileAllocated << endl;
	//atomicCounter.ResetCounter(0);

	//Store surface information ( Color, normal, etc. ) into the octree leaf nodes 
	leafStoreShader.use();
	leafStoreShader.setInt("u_level", levelOctree);
	leafStoreShader.setUInt("u_voxelDim", volumeDimension);
	leafStoreShader.setInt("u_numVoxel", numVoxel);
	glBindImageTexture(0, voxelPos.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGB10_A2UI);
	glBindImageTexture(1, voxelAlbedo.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	glBindImageTexture(2, octreeNode.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	glBindImageTexture(3, octreeNodeAlbedo.texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	//atomicCounter.ResetCounter(0);
	//glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter.bufId);
	//computeGroupDim = (numVoxel + 63) / 64;
	glDispatchCompute(groupDimX, groupDimY, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//Get the number of node tiles to allocate in the next level
	//tileAllocated = atomicCounter.GetCounter();
	//cout << "leaf flag : " << tileAllocated << endl;
	//atomicCounter.ResetCounter(0);
	float afterTime = glfwGetTime();
	//cout << "Total Time: " << 1 / (afterTime - beforTime) << endl;
	//if (true)
	//{
	//	//Get the buffer data
	//	glBindBuffer(GL_TEXTURE_BUFFER, octreeNodeAlbedo.tbo);
	//	unsigned int *output_image = new unsigned int[numOfNode];
	//	glGetBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(GLuint) * numOfNode, output_image);		
	//	int tempc = 0;
	//	int ind = 0;
	//	for (int i = 0; i < numOfNode; i++)
	//		if (output_image[i] != 0)
	//		{
	//			//if (i == 4888958) cout << "bingo" << endl;
	//			vec4 t = convRGBA8ToVec4(output_image[i]);
	//			tempc++;
	//			ind = i;
	//		}
	//	//cout << "tempc: " << tempc<<" ind:"<<ind << endl;
	//	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	//	delete[]output_image;
	//}
}

void VoxelRender::SVOTo3Dtex()
{
	atomicCounter.ResetCounter(0);
	while (glGetError());
	GLenum err;
	auto &octreeTo3DtexShader = AssetManager::getInstance()->shaderPrograms["octreeTo3Dtex"];
	octreeTo3DtexShader.use();
	octreeTo3DtexShader.setInt("u_level", levelOctree);
	octreeTo3DtexShader.setUInt("u_voxelDim", volumeDimension);
	glBindImageTexture(0, octree3D.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
	glBindImageTexture(1, octreeNode.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	glBindImageTexture(2, octreeNodeAlbedo.texId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounter.bufId);
	int computeGroupDim = (volumeDimension + 7) / 8;
	glDispatchCompute(computeGroupDim, computeGroupDim, computeGroupDim);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//cout << "atomicCounter: " << atomicCounter.GetCounter() << endl;
	
	////Get the buffer data
	//glBindBuffer(GL_TEXTURE_BUFFER, octreeNodeAlbedo.tbo);
	//unsigned int *output_image = new unsigned int[numOfNode];
	//glGetBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(GLuint)*numOfNode, output_image);
	//int count = 0;
	//for (int i = 0; i < numOfNode; i++)
	//{
	//	if (output_image[i] > 0)
	//	{
	//		int indx = i;
	//		uint icolor = output_image[i];
	//		vec4 vcolor = convRGBA8ToVec4(icolor);
	//		count++;
	//	}
	//}
	////cout << "albedo counts: " << count << endl;
	//glBindBuffer(GL_TEXTURE_BUFFER, 0);

	/*unsigned char *output_image = new unsigned char[volumeDimension*volumeDimension*volumeDimension*4];
	glBindTexture(GL_TEXTURE_3D, octree3D.id);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, output_image);
	int count = 0;
	unsigned int * oimage = (unsigned int *)output_image;
	int ind = 0;
	for (int i = 0; i < volumeDimension*volumeDimension*volumeDimension; ++i) {
		if (oimage[i]) {
			vec4 t = convRGBA8ToVec4(oimage[i]);
			ind = i;
			++count;
		}
	}
	cout <<"count:"<< count<<" ind:"<<ind << endl;;
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	delete[]output_image;*/
}

void VoxelRender::DrawSVO()
{

}
