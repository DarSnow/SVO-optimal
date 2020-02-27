#pragma once
#include "../PoissonSampleUtil/utils_sampling.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
using namespace std;

#ifndef GLEW_STATIC
#define GLEW_STATIC  
#endif // !GLEW_STATIC
#include <GL/glew.h> // Contains all the necessery OpenGL includes

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "../object/object.h"
#include "../object/camera.h"
#include "../AssetManager/AssetManager.h"
#include "../AssetManager/Loader.h"
#include "../object/light.h"

class PoissonSample {
public :

	static void SampleFromModel(Object* model) {
		vector<glm::vec3> vertexPos;
		vector<glm::vec3> vertexNor;
		vector<Mesh*> &meshes = AssetManager::getInstance()->meshes;
		for (int j = model->meshIndex.first; j < model->meshIndex.second; j++) {
			
			
		}
	}

	static void GenerateSamples(
		const size_t _VertexNumber,
		const glm::vec3* _VertexPositions,
		const glm::vec3* _VertexNormals,
		const size_t _TriangleNumber,
		const size_t* _TriangleIndex,   /** ด๓ะก3*TriangleNumber **/
		const int _ExpectSamplePointNumber,
		const float _ExpectMinRadiusBetweenSamplePoint,
		size_t &_SamplePointsNumber,
		glm::vec3 *&_SamplePointsPositionResult,
		glm::vec3 *&_SamplePointsNormalResult) {

		std::vector<Utils_sampling::Vec3> *Vert;
		std::vector<Utils_sampling::Vec3> *Normal;
		std::vector<int> *TriangleIndexs;
		std::vector<Utils_sampling::Vec3> *Samples_pos;
		std::vector<Utils_sampling::Vec3> *Samples_nor;

		Vert = new std::vector<Utils_sampling::Vec3>(_VertexNumber);
		Normal = new std::vector<Utils_sampling::Vec3>(_VertexNumber);
		TriangleIndexs = new std::vector<int>(_TriangleNumber * 3);
		Samples_pos = new std::vector<Utils_sampling::Vec3>;
		Samples_nor = new std::vector<Utils_sampling::Vec3>;

		for (size_t i = 0; i<_VertexNumber; i++)
		{
			(*Vert)[i] = Utils_sampling::Vec3(
				_VertexPositions[i].x, _VertexPositions[i].y, _VertexPositions[i].z);
			(*Normal)[i] = Utils_sampling::Vec3(
				_VertexNormals[i].x, _VertexNormals[i].y, _VertexNormals[i].z);
		}


		for (int i = 0; i<_TriangleNumber * 3; i++)
		{
			(*TriangleIndexs)[i] = _TriangleIndex[i];
		}

		Utils_sampling::poisson_disk(
			_ExpectMinRadiusBetweenSamplePoint,
			_ExpectSamplePointNumber,
			*Vert,
			*Normal,
			*TriangleIndexs,
			*Samples_pos,
			*Samples_nor);

		assert(Samples_pos->size()>0 && Samples_pos->size() == Samples_nor->size());

		delete (Vert);
		delete (Normal);
		delete (TriangleIndexs);


		_SamplePointsNumber = Samples_pos->size();
		_SamplePointsPositionResult = new glm::vec3[_SamplePointsNumber];
		_SamplePointsNormalResult = new glm::vec3[_SamplePointsNumber];


		for (size_t i = 0; i<_SamplePointsNumber; i++)
		{
			Utils_sampling::Vec3 Pos = (*Samples_pos)[i];
			Utils_sampling::Vec3 Nor = (*Samples_nor)[i];
			_SamplePointsPositionResult[i] = glm::vec3(Pos.x, Pos.y, Pos.z);
			_SamplePointsNormalResult[i] = glm::vec3(Nor.x, Nor.y, Nor.z);
		}

		delete Samples_pos;
		delete Samples_nor;
	}
};