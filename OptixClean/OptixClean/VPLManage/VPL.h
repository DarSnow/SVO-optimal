#pragma once
#include <optixu/optixu_vector_types.h>
#include <cmath>
#include <time.h>

struct VPL {
	optix::float3 pos;
	optix::float3 normal;
	optix::float3 color;
	optix::float3 radiosity;
	optix::float3 pdf;
	unsigned seed;
	VPL(optix::float3 ipos, optix::float3 inormal, optix::float3 icolor) :
		pos(ipos), normal(inormal), color(icolor)
	{
		seed = std::rand();
	}
	VPL() {}
};