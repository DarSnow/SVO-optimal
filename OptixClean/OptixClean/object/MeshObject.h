#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "../Common/GLheader.h"
#include "Transform.h"
#include "../AssetManager/AssetManager.h"
#include "Object.h"

class MeshObject : public SceneObject {
public:
	/// <summary>
	/// The object's transform
	/// </summary>
	pair<int, int> meshIndex;
	MeshObject(string _name = "") { }
	~MeshObject() { }

	MeshObject operator= (const MeshObject &obj) {
		this->name = obj.name;
		this->meshIndex = obj.meshIndex;
		this->transform = obj.transform;
		return *this;
	}

	mat4 getModelMat() {
		return transform.Matrix();
	}
};

class ObjectManager : public Singleton<ObjectManager> {
public:
	ObjectManager() {}
	void readConfig(string config); 
	void ObjectManager::drawObject(string name, GLuint shader);
	mat4 ObjectManager::getModel(string name);
	map<string, MeshObject> objects;
};
