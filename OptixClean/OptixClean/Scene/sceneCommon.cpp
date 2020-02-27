#include "scene.h"
#include "../optixPart/optixSampler.h"
#include <math.h>

//画两个地图
void Scene::draw2D() {
	//固定管线
	glUseProgram(0);
	vec4 pos, dir;
	//分割线
	glViewport(0, 0, width, height);
	map2d::drawLine(vec2(1.0f / 3, 0), vec2(1.0f, 0), vec3(0.3, 0.6, 0), 5);
	map2d::drawLine(vec2(1.0f / 3, -1.0f), vec2(1.0f / 3, -1.0f), vec3(0.3, 0.6, 0), 5);
	glViewport(width * 2 / 3, height / 2, width / 3, height / 2);

	map2d::drawVirtul();
	pos = (spcamera.getVirtulPos());
	pos += vec4(-1.9f, 0, 1.9f, 0);
	pos /= 4.0f * pos.w;
	map2d::drawPoint(pos.x, pos.z, 10.0f);
	dir = (pos + spcamera.getVirtulDir()*0.15f);
	//朝向
	map2d::drawLinv(vec2(pos.x, pos.z), vec2(dir.x, dir.z), vec3(0.8, 1.0, 0.5));

	//下面的地图
	glViewport(width * 2 / 3, 0, width / 3, height / 2);

	map2d::drawReal();
	pos = (spcamera.getRealPos());
	pos *= 0.4 / pos.w;
	map2d::drawPoint(pos.x, pos.z, 20.0f);
	dir = (pos + spcamera.getRealDir()*0.3f);
	//方向
	map2d::drawLinv(vec2(pos.x, pos.z), vec2(dir.x, dir.z), vec3(1.0, 1.0, 0.5));

}

bool Scene::handleInput() {
	if (!progressMessage()) return false;

#ifdef HASVR
	vr.HandleInput();
	camera.updateView(vr.HMDPoseInverse);
#else
	if (state['Q'])camera.rollLeft(0.015);
	if (state['E'])camera.rollLeft(-0.015);
	if (state['X'])camera.rollUp(0.015);
	if (state['C'])camera.rollUp(-0.015);
	if (state['W'])camera.move(0.015, 0);
	if (state['S'])camera.move(-0.015, 0);
	if (state['A'])camera.move(0, 0.015);
	if (state['D'])camera.move(0, -0.015);

	if (state['I'])light[0].move(0, 0, 0.1);
	if (state['J'])light[0].move(0, 0, -0.1);
	if (state['K'])light[0].move(0, 0.1);
	if (state['L'])light[0].move(0, -0.1);
	if (state['U'])light[0].move(0.1, 0);
	if (state['O'])light[0].move(-0.1, 0);
#endif
	frameNum++;

	return true;
}

bool Scene::progressMessage() {
	while (!window->messagePump.empty()) {
		WindowCallBack::MyWindow::Message m = window->messagePump.front();
		window->messagePump.pop();
		if (m.type == WindowCallBack::MyWindow::Message::keybordPress) {
			switch (m.value.value) {
			case GLFW_KEY_ESCAPE:
				return false;
				break;
			case GLFW_KEY_SPACE:
				state[SPACE] = true;
				break;

			}
			if (m.value.value >= GLFW_KEY_0 && m.value.value <= GLFW_KEY_9) {
				state['0' + m.value.value - GLFW_KEY_0] = true;
			}
			else if (m.value.value >= GLFW_KEY_A && m.value.value <= GLFW_KEY_Z) {
				state['A' + m.value.value - GLFW_KEY_A] = true;
			}
		}
		if (m.type == WindowCallBack::MyWindow::Message::keybordRelese) {
			switch (m.value.value) {
			case GLFW_KEY_SPACE:
				state[SPACE] = false;
				break;
			}
			if (m.value.value >= GLFW_KEY_0 && m.value.value <= GLFW_KEY_9) {
				state['0' + m.value.value - GLFW_KEY_0] = false;
			}
			else if (m.value.value >= GLFW_KEY_A && m.value.value <= GLFW_KEY_Z) {
				state['A' + m.value.value - GLFW_KEY_A] = false;
			}
		}
	}
	return true;
}