#pragma once
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

#include <sutil.h>
#include "commonStructs.h"
#include "random.h"
#include "MeshBuilder.h"
#include <Arcball.h>
#include <OptiXMesh.h>

#include <cassert>
#include <string>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
using namespace optix;

class DynamicLayout
{
public:
	virtual ~DynamicLayout() {}
	virtual void createGeometry(Context ctx) = 0;
	virtual void addGeometry(Context ctx, GeometryInstance obj, Matrix4x4 loadform = Matrix4x4::identity()) = 0;
	virtual void addGeometry(Context ctx, const std::string& filename, Matrix4x4 loadform) = 0;
	virtual void addMultiGeometry(Context ctx, const std::string& filename, int num_meshes) = 0;
	virtual void endGeometry(Context ctx) = 0;
	virtual void setCtxGeometry(Context ctx) = 0;
	virtual Aabb getSceneBBox() const = 0;
	virtual void triggerGeometryMove() = 0;
	virtual void updateGeometry() = 0;
	virtual void resetGeometry() = 0;
};

class SeparateAccelsLayout : public DynamicLayout
{
public:
	SeparateAccelsLayout(const std::string& builder, bool print_timing);

	void createGeometry(Context ctx);
	void addGeometry(Context ctx, GeometryInstance obj, Matrix4x4 loadform = Matrix4x4::identity());
	void addGeometry(Context ctx, const std::string& filename, Matrix4x4 loadform);
	void addMultiGeometry(Context ctx, const std::string& filename, int num_meshes);
	void endGeometry(Context ctx);
	void setCtxGeometry(Context ctx);
	Aabb getSceneBBox()const;
	void triggerGeometryMove();
	void updateGeometry();
	void resetGeometry();

private:
	struct Mesh
	{
		std::string name;
		Transform xform;
		float3    start_pos;
		float3    end_pos;
		double    move_start_time;
		bool is_dynamic;
	};

	Aabb                    m_aabb;
	std::vector<Mesh>       m_meshes;
	int                     m_num_moved_meshes;
	Group                   m_top_object;
	std::string             m_builder;
	bool                    m_print_timing;
};


inline float3 getRandomPosition(int child_idx, int position_idx)
{
	unsigned seed = tea<4>(child_idx, position_idx);
	const float f0 = rnd(seed) - 0.5f;
	const float f1 = rnd(seed) - 0.5f;
	const float f2 = rnd(seed) - 0.5f;
	return make_float3(f0, f1, f2);
}

const float MOVE_ANIMATION_TIME = 10.0f;