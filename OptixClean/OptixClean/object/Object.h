#pragma once
#include "Transform.h"
#include <string>

class SceneObject : public GLTransform{
public:

	GLTransform &transform;
	std::string name;

	SceneObject() : transform(*this) {}
	~SceneObject() { }

	SceneObject(const SceneObject &obj) : transform(*this) {
		this->name = obj.name;
	}
};