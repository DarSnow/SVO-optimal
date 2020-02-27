#pragma once
#include "../Common/GLheader.h"
#include <GLFW\glfw3.h>

template<class T>
class Twin {
public:
	Twin(double duration, T begin, T end) {
		this->duration = duration;
		this->begin = begin;
		this->end = end;
		this->startTime = glfwGetTime();
	}

	T update() {
		return (this->end - this->begin)*(glfwGetTime() - startTime) / duration;
	}

private:
	double duration;
	double startTime;
	T begin;
	T end;
};