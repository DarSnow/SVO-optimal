#pragma once
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <vector>
#include <cmath>
#include "../optixPart/optixSampler.h"
using namespace std;

// filter invalid sample pos
void updateSamplePoints(OptixSampler* opSamp, GLuint &selectedNum, float* selectedSamplePointsPos, float* selectedSamplePointsNor, float* selectedSamplePointsCol)
{
	float* sampos = opSamp->samplePosArr;
	float* samnor = opSamp->sampleNorArr;
	float* samcol = opSamp->sampleClrArr;
	selectedNum = 0;
	int ele_width = 4;
	glm::uint arrlen = opSamp->samplenum * ele_width * 3;
	for (glm::uint i = 0; i < arrlen; i+=4) {
		if (!(abs(sampos[i]) < 0.1f && abs(sampos[i + 1]) < 0.1f && abs(sampos[i + 2]) < 0.1f)) {
			int sidx = selectedNum * ele_width;
			memcpy(selectedSamplePointsPos + sidx, sampos + i, ele_width * sizeof(float));
			memcpy(selectedSamplePointsNor + sidx, samnor + i, ele_width * sizeof(float));
			memcpy(selectedSamplePointsCol + sidx, samcol + i, ele_width * sizeof(float));
			selectedNum += 1;
		}
	}
}


void makeStatistics(int vplNum, float *samplePointsPos) {
	cout << "vplNum: " << vplNum << endl;
	cout << "samplePointsPos[3 * i + 1]: " << endl;
	vector <float> y;
	float min = 5.0;
	float max = 0.0;
	int count = 0, countmin = 0;
	for (int i = 0; i < vplNum; i++)
	{
		if (samplePointsPos[3 * i + 1] > max)
		{
			max = samplePointsPos[3 * i + 1];
			count = 0;
		}
		else if (samplePointsPos[3 * i + 1] == max)
		{
			count++;
		}
		else if (samplePointsPos[3 * i + 1] < min)
		{
			min = samplePointsPos[3 * i + 1];
			countmin = 0;
		}
		else if (samplePointsPos[3 * i + 1] == max)
		{
			countmin++;
		}
		y.push_back(samplePointsPos[3 * i + 1]);
		cout << samplePointsPos[3 * i + 1] << endl;
	}
	cout << "max: " << max << endl;
	cout << "count: " << count << endl;
	cout << "min: " << min << endl;
	cout << "countmin: " << countmin << endl;
}

void sampleFromeWall(int selectedNum, int vplNum, vector<int> selectedVPL, 
	float *samplePointsPos, float *samplePointsNor, float *samplePointsCol,
	float *selectedSamplePointsPos, float *selectedSamplePointsNor, float *selectedSamplePointsCol) {
	srand((unsigned)time(NULL));
	// 取绿墙10
	for (int i = 0; i < selectedNum - 40; i++)
	{
		//随机数函数

		int random = rand() % vplNum - 1;
		vector<int>::iterator ret;
		ret = std::find(selectedVPL.begin(), selectedVPL.end(), random);
		//y的最小值-2.16573是手动选的
		//if ((ret == selectedVPL.end()) && (samplePointsPos[3 * random + 1] > -2.16573))
		//想取绿墙面上的vpl
		if ((ret == selectedVPL.end()) && (samplePointsPos[3 * random] > 3.7) && (samplePointsPos[3 * random + 1] > -2.25) && (samplePointsPos[3 * random + 1] < 5.3) && (samplePointsPos[3 * random + 2] < 10.8))
		{
			//cout << "not found" << endl;
			selectedVPL.push_back(random);
			//selectedVPL[i] = random;
			selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
			selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
			selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
			selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
			selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
			selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
			selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
			selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
			selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
			//cout << "randomSamplePointsCol.r: " << samplePointsCol[3 * random] << endl;
			//cout << "randomSamplePointsCol.g: " << samplePointsCol[3 * random + 1] << endl;
			//cout << "randomSamplePointsCol.b: " << samplePointsCol[3 * random + 2] << endl;
			//cout << "random: " << random << endl;
		}
		else
		{
			i--;
		}
	}
	// 取红墙10
	for (int i = 10; i < selectedNum - 30; i++)
	{
		//随机数函数
		int random = rand() % vplNum - 1;
		vector<int>::iterator ret;
		ret = std::find(selectedVPL.begin(), selectedVPL.end(), random);
		if ((ret == selectedVPL.end()) && (samplePointsPos[3 * random] < -3.7) && (samplePointsPos[3 * random + 1] > -2.16573) && (samplePointsPos[3 * random + 1] < 5.3) && (samplePointsPos[3 * random + 2] < 10.8))
		{
			selectedVPL.push_back(random);
			selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
			selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
			selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
			selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
			selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
			selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
			selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
			selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
			selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
			//cout << "random: " << random << endl;
		}
		else
		{
			i--;
		}
	}
	// 取后墙10
	for (int i = 20; i < selectedNum - 20; i++)
	{
		//随机数函数
		int random = rand() % vplNum - 1;
		vector<int>::iterator ret;
		ret = std::find(selectedVPL.begin(), selectedVPL.end(), random);
		if ((ret == selectedVPL.end()) && (samplePointsPos[3 * random] > -3.6) && (samplePointsPos[3 * random] < 3.9) && (samplePointsPos[3 * random + 1] > -2.16573) && (samplePointsPos[3 * random + 1] < 5.3) && (samplePointsPos[3 * random + 2] > 10.97))
		{
			selectedVPL.push_back(random);
			selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
			selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
			selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
			selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
			selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
			selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
			selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
			selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
			selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
			//cout << "random: " << random << endl;
		}
		else
		{
			i--;
		}
	}

	// 取立方体15
	//{
	//	int i = 28;
	//	int random = 380;
	//	selectedVPL.push_back(random);
	//	selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
	//	selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
	//	selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
	//	selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
	//	selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
	//	selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
	//	selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
	//	selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
	//	selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
	//}
	//{
	//	int i = 29;
	//	int random = 386;
	//	selectedVPL.push_back(random);
	//	selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
	//	selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
	//	selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
	//	selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
	//	selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
	//	selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
	//	selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
	//	selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
	//	selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
	//}

	// 取地板5，大于30的map放在另外的数组里
	for (int i = 30; i < selectedNum - 15; i++)
	{
		//随机数函数
		int random = rand() % vplNum - 1;
		vector<int>::iterator ret;
		ret = std::find(selectedVPL.begin(), selectedVPL.end(), random);
		if ((ret == selectedVPL.end()) && (samplePointsPos[3 * random] > -3.6) && (samplePointsPos[3 * random] < 3.9) && (samplePointsPos[3 * random + 1] < -2.16573) && (samplePointsPos[3 * random + 2] < 10.97))
		{
			selectedVPL.push_back(random);
			selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
			selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
			selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
			selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
			selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
			selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
			selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
			selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
			selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
		}
		else
		{
			i--;
		}
	}

	// 取立方体15
	for (int i = 35; i < selectedNum; i++)
	{
		//随机数函数
		int random = rand() % vplNum - 1;
		vector<int>::iterator ret;
		ret = std::find(selectedVPL.begin(), selectedVPL.end(), random);
		if ((ret == selectedVPL.end()) && (samplePointsPos[3 * random] > -3.6) && (samplePointsPos[3 * random] < 3.9) && (samplePointsPos[3 * random + 1] > -2.16573) && (samplePointsPos[3 * random + 1] < 5.3) && (samplePointsPos[3 * random + 2] < 10.97))
		{
			selectedVPL.push_back(random);
			selectedSamplePointsPos[3 * i] = samplePointsPos[3 * random];
			selectedSamplePointsPos[3 * i + 1] = samplePointsPos[3 * random + 1];
			selectedSamplePointsPos[3 * i + 2] = samplePointsPos[3 * random + 2];
			selectedSamplePointsNor[3 * i] = samplePointsNor[3 * random];
			selectedSamplePointsNor[3 * i + 1] = samplePointsNor[3 * random + 1];
			selectedSamplePointsNor[3 * i + 2] = samplePointsNor[3 * random + 2];
			selectedSamplePointsCol[3 * i] = samplePointsCol[3 * random];
			selectedSamplePointsCol[3 * i + 1] = samplePointsCol[3 * random + 1];
			selectedSamplePointsCol[3 * i + 2] = samplePointsCol[3 * random + 2];
		}
		else
		{
			i--;
		}
	}

	//cout << selectedVPL.size() << endl;
	/*for (int i = 0; i < selectedNum; i++)
	{
	cout << "selectedSamplePointsPos.x: " << selectedSamplePointsPos[3 * i] << endl;
	cout << "selectedSamplePointsPos.y: " << selectedSamplePointsPos[3 * i + 1] << endl;
	cout << "selectedSamplePointsPos.z: " << selectedSamplePointsPos[3 * i + 2] << endl;
	cout << "selectedSamplePointsNor.x: " << selectedSamplePointsNor[3 * i] << endl;
	cout << "selectedSamplePointsNor.y: " << selectedSamplePointsNor[3 * i + 1] << endl;
	cout << "selectedSamplePointsNor.z: " << selectedSamplePointsNor[3 * i + 2] << endl;
	cout << "selectedSamplePointsCol.r: " << selectedSamplePointsCol[3 * i] << endl;
	cout << "selectedSamplePointsCol.g: " << selectedSamplePointsCol[3 * i + 1] << endl;
	cout << "selectedSamplePointsCol.b: " << selectedSamplePointsCol[3 * i + 2] << endl;
	}
	*/
}