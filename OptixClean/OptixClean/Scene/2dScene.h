#pragma once

#include "../Common/GLheader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#define V_WALL_NUM 12
#define BOUNDARY 0.8f
namespace map2d {

	vec2 vWall[];


	void drawPoint(float x, float y, float r);
	void drawLinv(vec2 p1, vec2 p2, vec3 color);
	void drawLine(vec2 p1, vec2 p2, vec3 color ,float lineWidth = 2);
	void drawReal();
	void drawVirtul();
}