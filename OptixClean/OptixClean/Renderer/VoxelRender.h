#pragma once
#include "../ModelClass/Texture.h"
#include "../object/BoundingBox.h"
using namespace ModelClass;

class VoxelRender {
public:
	VoxelRender();
	virtual void Render();
	void SetupGlobalUniform(BoundingBox sceneBox);
	void VoxelizeScene(int isStore);
	void InjectRadiance();
	void DrawVoxels();
	void SetupVoxelVolumes(const unsigned int &dimension);
	void BuildSVO();
	void SVOTo3Dtex();
	void DrawSVO();
	void testImageStore();
private:
	TextureBuffer voxelPos;
	TextureBuffer voxelAlbedo;
	TextureBuffer voxelNormal;
	TextureBuffer voxelEmissive;
	TextureBuffer voxelRadiance;
	TextureBuffer octreeNode;	//This is for storing child node indices
	TextureBuffer octreeNodeAlbedo;	//This is for storing diffuse 
	Texture3D octree3D;	//This is for show the SVO
	AtomicBuffer atomicCounter;
	
	std::array<Texture3D, 6> voxelTexMipmap;
	// vertex buffer object for 3d texture visualization
	GLuint voxelDrawerArray;
	std::array<glm::mat4x4, 3> viewProjectionMatrix;
	std::array<glm::mat4x4, 3> viewProjectionMatrixI;
	unsigned int volumeDimension;
	unsigned int voxelCount;	//all voxels include empty
	unsigned int numVoxel;	//the number of useful voxels
	unsigned int numOfNode;	//the octree buffer size
	int levelOctree;
	bool injectFirstBounce;
	float volumeGridSize;
	float voxelSize;
	int framestep;
	bool traceShadowCones;
	bool normalWeightedLambert;
	float traceShadowHit;

	unsigned int drawMipLevel;
	unsigned int drawDirection;
	glm::vec4 drawColorChannels;

	bool initFlag = false;
 };