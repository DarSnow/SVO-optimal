#pragma once
#include "VPL.h"

class VPLManager {
public:
	void update(int selectedNum, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol);

	VPLManager(int capacity = 1000);
	~VPLManager();
private:
	VPL *lastvplarr;
	VPL *curvplarr;
};
