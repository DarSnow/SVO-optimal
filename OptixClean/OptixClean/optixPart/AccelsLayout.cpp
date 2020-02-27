#include "AccelsLayout.h"

SeparateAccelsLayout::SeparateAccelsLayout(const std::string& builder, bool print_timing)
	: m_num_moved_meshes(0), m_builder(builder), m_print_timing(print_timing)
{
}


void SeparateAccelsLayout::createGeometry(Context ctx)
{
	std::cerr << "Creating geometry ... ";

	m_top_object = ctx->createGroup();
	m_top_object->setAcceleration(ctx->createAcceleration(m_builder.c_str()));

	assert(m_meshes.size() == 0);	
}

void SeparateAccelsLayout::addGeometry(Context ctx, GeometryInstance obj, Matrix4x4 loadform) {
	GeometryGroup geometry_group = ctx->createGeometryGroup();
	geometry_group->setAcceleration(ctx->createAcceleration(m_builder.c_str()));
	geometry_group->addChild(obj);

	Transform xform = ctx->createTransform();
	xform->setMatrix(false, loadform.getData(), 0);
	xform->setChild(geometry_group);
	m_top_object->addChild(xform);
	
	Mesh mesh;
	mesh.xform = xform;
	mesh.move_start_time = 0.0;
	m_meshes.push_back(mesh);
}

void SeparateAccelsLayout::addGeometry(Context ctx, const std::string& filename, Matrix4x4 loadform) {

	OptiXMesh omesh;
	omesh.context = ctx;
	OptixMeshSpace::loadMesh(filename, omesh);

	GeometryGroup geometry_group = ctx->createGeometryGroup();
	geometry_group->setAcceleration(ctx->createAcceleration(m_builder.c_str()));
	geometry_group->addChild(omesh.geom_instance);

	Transform xform = ctx->createTransform();
	xform->setMatrix(false, loadform.getData(), 0);
	xform->setChild(geometry_group);
	m_top_object->addChild(xform);

	Mesh mesh;
	mesh.xform = xform;
	mesh.move_start_time = 0.0;
	mesh.is_dynamic = false;
	m_meshes.push_back(mesh);
}

void SeparateAccelsLayout::addMultiGeometry(Context ctx, const std::string& filename, int num_meshes) {
	
	for (int i = 0; i < num_meshes; ++i)
	{
		OptiXMesh omesh;
		omesh.context = ctx;
		OptixMeshSpace::loadMesh(filename, omesh);
		m_aabb = Aabb(omesh.bbox_min, omesh.bbox_max);
		const float max_dim = fmaxf(m_aabb.extent(0), m_aabb.extent(1));

		GeometryGroup geometry_group = ctx->createGeometryGroup();
		geometry_group->setAcceleration(ctx->createAcceleration(m_builder.c_str()));
		geometry_group->addChild(omesh.geom_instance);

		float3 pos0 = getRandomPosition(i, 0)*max_dim * 10;
		float3 pos1 = getRandomPosition(i, 1)*max_dim * 10;

		Transform xform = ctx->createTransform();

		xform->setMatrix(false, Matrix4x4::translate(pos0).getData(), 0);

		xform->setChild(geometry_group);
		m_top_object->addChild(xform);

		Mesh mesh;
		mesh.xform = xform;
		mesh.start_pos = pos0;
		mesh.end_pos = pos1;
		mesh.is_dynamic = true;
		mesh.move_start_time = sutil::currentTime();
		m_meshes.push_back(mesh);
	}
}

void SeparateAccelsLayout::endGeometry(Context ctx) {
	ctx["top_object"]->set(m_top_object);
	ctx["top_shadower"]->set(m_top_object);
	std::cerr << "done" << std::endl;;
}

void SeparateAccelsLayout::setCtxGeometry(Context ctx) {
	ctx["top_object"]->set(m_top_object);
	ctx["top_shadower"]->set(m_top_object);
}

Aabb SeparateAccelsLayout::getSceneBBox()const
{
	return m_aabb;
}


void SeparateAccelsLayout::triggerGeometryMove()
{
	int size = static_cast<int>(m_meshes.size());
	
	for (int i = 0; i<size; ++i)
	{
		const double cur_time = sutil::currentTime();
		m_meshes[m_num_moved_meshes].move_start_time = cur_time;
		++m_num_moved_meshes;
	}
}

void SeparateAccelsLayout::updateGeometry()
{
	double t0 = sutil::currentTime();

	bool meshes_have_moved = false;
	assert(m_num_moved_meshes <= static_cast<int>(m_meshes.size()));
	for (int i = 0; i < m_num_moved_meshes; ++i)
	{
		Mesh mesh = m_meshes[i];
		const double elapsed_time = t0 - mesh.move_start_time;
		if (elapsed_time >= MOVE_ANIMATION_TIME || !mesh.is_dynamic)
			continue;

		const float t = static_cast<float>(elapsed_time / MOVE_ANIMATION_TIME);
		const float3 pos = lerp(mesh.start_pos, mesh.end_pos, t);
		mesh.xform->setMatrix(false, Matrix4x4::translate(pos).getData(), 0);
		meshes_have_moved = true;
	}

	double t1 = sutil::currentTime();

	if (meshes_have_moved)
	{
		if (m_print_timing)
		{
			std::cerr << "Geometry transform time: "
				<< std::fixed << std::setw(7) << std::setprecision(2) << (t1 - t0)*1000.0 << "ms" << std::endl;
		}
		t0 = sutil::currentTime();
		m_top_object->getAcceleration()->markDirty();
		m_top_object->getContext()->launch(0, 0, 0);
		t1 = sutil::currentTime();
		if (m_print_timing)
		{
			std::cerr << "Accel rebuild time     : "
				<< std::fixed << std::setw(7) << std::setprecision(2) << (t1 - t0)*1000.0 << "ms" << std::endl;
		}
	}

}


void SeparateAccelsLayout::resetGeometry()
{
	if (m_num_moved_meshes == 0)
		return;

	assert(m_num_moved_meshes <= static_cast<int>(m_meshes.size()));
	for (int i = 0; i < m_num_moved_meshes; ++i)
	{
		Mesh mesh = m_meshes[i];
		mesh.xform->setMatrix(false, Matrix4x4::translate(mesh.start_pos).getData(), 0);
	}

	m_num_moved_meshes = 0;
	m_top_object->getAcceleration()->markDirty();
}