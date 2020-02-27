#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "../Common/GLheader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../ModelClass/Shader.h"
#include "../ModelClass/meterial.h"
#include "../ModelClass/mesh.h"
#include "../ModelClass/Texture.h"
#include "../Common/Singleton.h"
using namespace ModelClass;

class AssetManager: public Singleton<AssetManager>{
public:
	//��������Ϣ���������õ���ģ�͡����ʡ���ͼ��������ɫ��
	void readConfig(string configStream);
	//����ģ��
	void loadMesh(string name, string path, bool isLoadMeterial);

private:
	void processNode(string path, aiNode* node, const aiScene* scene, bool isLoadMeterial);
	void processMesh(string path, aiMesh* mesh, const aiScene* scene, bool isLoadMeterial);
	void loadMeterial(Meterial &m);
	
public:
	//����
	vector<ModelClass::GLMesh*> meshes;
	//ÿ��obj�ļ�һ��group
	map<string ,pair<int, int> > meshGroups;

	//��ɫ��������-����
	map<string, Shader> shaderPrograms;

	//���ʱ�,��ͼΨһ��������Ψһ
	map<string,Meterial> meterials;
	map<string, Texture> textures;
};