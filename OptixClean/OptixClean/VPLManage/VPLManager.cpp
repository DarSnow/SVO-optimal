#include "VPLManager.h"

void VPLManager::update(int selectedNum, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol) {

}

VPLManager::VPLManager(int capacity) {
	lastvplarr = new VPL[capacity];
	curvplarr = new VPL[capacity];
}

VPLManager::~VPLManager() {
	delete []lastvplarr;
	delete []curvplarr;
}