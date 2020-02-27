#pragma once
#include "../Common/GLheader.h"

#include <optixu/optixu_math_namespace.h>

#include <Mesh.h>
#include <OptiXMesh.h>
#include <sutil.h>
#include "ComConfig.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace optix {
	static float3 make_float3(const float* a)
	{
		return make_float3(a[0], a[1], a[2]);
	}
}

namespace OptixMeshSpace
{
	struct MeshBuffers
	{
		optix::Buffer tri_indices;
		optix::Buffer mat_indices;
		optix::Buffer positions;
		optix::Buffer normals;
		optix::Buffer texcoords;
	};

	static optix::Buffer createArrBuffer(
		optix::Context context,
		RTformat format,
		unsigned width,
		bool use_pbo,
		RTbuffertype buffer_type)
	{
		optix::Buffer buffer;
		if (use_pbo)
		{
			// First allocate the memory for the GL buffer, then attach it to OptiX.

			// Assume ubyte4 or float4 for now
			unsigned int elmt_size = format == RT_FORMAT_UNSIGNED_BYTE4 ? 4 : 16;

			GLuint vbo = 0;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, elmt_size * width, 0, GL_STREAM_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			buffer = context->createBufferFromGLBO(buffer_type, vbo);
			buffer->setFormat(format);
			buffer->setSize(width);
		}
		else
		{
			buffer = context->createBuffer(buffer_type, format, width);
		}

		return buffer;
	}

	static void setupMeshLoaderInputs(
		optix::Context            context,
		MeshBuffers&              buffers,
		Mesh&                     mesh
	)
	{
		buffers.tri_indices = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, mesh.num_triangles);
		buffers.mat_indices = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT, mesh.num_triangles);
		buffers.positions = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, mesh.num_vertices);
		buffers.normals = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3,
			mesh.has_normals ? mesh.num_vertices : 0);
		buffers.texcoords = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2,
			mesh.has_texcoords ? mesh.num_vertices : 0);

		mesh.tri_indices = reinterpret_cast<int32_t*>(buffers.tri_indices->map());
		mesh.mat_indices = reinterpret_cast<int32_t*>(buffers.mat_indices->map());
		mesh.positions = reinterpret_cast<float*>  (buffers.positions->map());
		mesh.normals = reinterpret_cast<float*>  (mesh.has_normals ? buffers.normals->map() : 0);
		mesh.texcoords = reinterpret_cast<float*>  (mesh.has_texcoords ? buffers.texcoords->map() : 0);

		mesh.mat_params = new MaterialParams[mesh.num_materials];
	}


	static void unmap(MeshBuffers& buffers, Mesh& mesh)
	{
		buffers.tri_indices->unmap();
		buffers.mat_indices->unmap();
		buffers.positions->unmap();
		if (mesh.has_normals)
			buffers.normals->unmap();
		if (mesh.has_texcoords)
			buffers.texcoords->unmap();

		mesh.tri_indices = 0;
		mesh.mat_indices = 0;
		mesh.positions = 0;
		mesh.normals = 0;
		mesh.texcoords = 0;

		delete[] mesh.mat_params;
		mesh.mat_params = 0;
	}


	static void createMaterialPrograms(
		optix::Context         context,
		bool                   use_textures,
		optix::Program&        closest_hit,
		optix::Program&        any_hit
	)
	{
		const char *ptx = sutil::getPtxString(NULL, myphonecu_file.c_str());

		if (!closest_hit)
			closest_hit = context->createProgramFromPTXString(ptx, use_textures ? "closest_hit_radiance_textured" : "closest_hit_radiance");
		if (!any_hit)
			any_hit = context->createProgramFromPTXString(ptx, "any_hit_shadow");
	}


	static optix::Material createOptiXMaterial(
		optix::Context         context,
		optix::Program         closest_hit,
		optix::Program         any_hit,
		optix::Program         sample_closest_hit,
		optix::Program		   samplelight_closest_hit,
		optix::Program         diffuse_ch,
		optix::Program         diffuse_ah,
		const MaterialParams&  mat_params,
		bool                   use_textures
	)
	{
		optix::Material mat = context->createMaterial();
		mat->setClosestHitProgram(0u, closest_hit);
		mat->setAnyHitProgram(1u, any_hit);
		mat->setClosestHitProgram(2u, sample_closest_hit);
		mat->setClosestHitProgram(3u, samplelight_closest_hit);
		mat->setClosestHitProgram(4u, diffuse_ch);
		mat->setAnyHitProgram(5u, diffuse_ah);

		if (use_textures)
			mat["Kd_map"]->setTextureSampler(sutil::loadTexture(context, mat_params.Kd_map, optix::make_float3(mat_params.Kd)));
		else
			mat["Kd_map"]->setTextureSampler(sutil::loadTexture(context, "", optix::make_float3(mat_params.Kd)));

		mat["Kd_mapped"]->setInt(use_textures);
		mat["Kd"]->set3fv(mat_params.Kd);
		mat["Ks"]->set3fv(mat_params.Ks);
		mat["Kr"]->set3fv(mat_params.Kr);
		mat["Ka"]->set3fv(mat_params.Ka);
		mat["phong_exp"]->setFloat(mat_params.exp);

		return mat;
	}


	static optix::Program createBoundingBoxProgram(optix::Context context)
	{
		return context->createProgramFromPTXString(sutil::getPtxString(NULL, trianglecu_file.c_str()), "mesh_bounds");
	}


	static optix::Program createIntersectionProgram(optix::Context context)
	{
		return context->createProgramFromPTXString(sutil::getPtxString(NULL, trianglecu_file.c_str()), "mesh_intersect");
	}


	static void translateMeshToOptiX(
		const Mesh&        mesh,
		const MeshBuffers& buffers,
		OptiXMesh&         optix_mesh
	)
	{
		optix::Context ctx = optix_mesh.context;
		optix_mesh.bbox_min = optix::make_float3(mesh.bbox_min);
		optix_mesh.bbox_max = optix::make_float3(mesh.bbox_max);
		optix_mesh.num_triangles = mesh.num_triangles;

		std::vector<optix::Material> optix_materials;
		if (optix_mesh.material)
		{
			// Rewrite all mat_indices to point to single override material
			memset(mesh.mat_indices, 0, mesh.num_triangles * sizeof(int32_t));

			optix_materials.push_back(optix_mesh.material);
		}
		else
		{
			bool have_textures = false;
			for (int32_t i = 0; i < mesh.num_materials; ++i)
				if (!mesh.mat_params[i].Kd_map.empty())
					have_textures = true;

			optix::Program closest_hit = optix_mesh.closest_hit;
			optix::Program any_hit = optix_mesh.any_hit;
			createMaterialPrograms(ctx, have_textures, closest_hit, any_hit);
			//sample closest hit function
			const char *sampleptx = sutil::getPtxString(NULL, samplecu_file.c_str());
			optix::Program sample_closest_hit = ctx->createProgramFromPTXString(sampleptx, "closest_hit_sample");
			optix::Program samplelight_closest_hit = ctx->createProgramFromPTXString(sampleptx, "closest_hit_samplelight");
			
			const char *pathtrcptx = sutil::getPtxString(NULL, pathtracecu_file.c_str());
			optix::Program diffuse_ch = ctx->createProgramFromPTXString(pathtrcptx, "diffuse");
			optix::Program diffuse_ah = ctx->createProgramFromPTXString(pathtrcptx, "shadow");

			for (int32_t i = 0; i < mesh.num_materials; ++i)
				optix_materials.push_back(createOptiXMaterial(
					ctx,
					closest_hit,
					any_hit,
					sample_closest_hit,
					samplelight_closest_hit,
					diffuse_ch,
					diffuse_ah,
					mesh.mat_params[i],
					have_textures));
		}

		optix::Geometry geometry = ctx->createGeometry();
		geometry["vertex_buffer"]->setBuffer(buffers.positions);
		geometry["normal_buffer"]->setBuffer(buffers.normals);
		geometry["texcoord_buffer"]->setBuffer(buffers.texcoords);
		geometry["material_buffer"]->setBuffer(buffers.mat_indices);
		geometry["index_buffer"]->setBuffer(buffers.tri_indices);
		geometry->setPrimitiveCount(mesh.num_triangles);
		geometry->setBoundingBoxProgram(optix_mesh.bounds ?
			optix_mesh.bounds :
			createBoundingBoxProgram(ctx));
		geometry->setIntersectionProgram(optix_mesh.intersection ?
			optix_mesh.intersection :
			createIntersectionProgram(ctx));

		optix_mesh.geom_instance = ctx->createGeometryInstance(
			geometry,
			optix_materials.begin(),
			optix_materials.end()
		);
	}


	static void loadMesh(
		const std::string&          filename,
		OptiXMesh&                  optix_mesh,
		const optix::Matrix4x4&     load_xform = optix::Matrix4x4::identity()
	)
	{
		if (!optix_mesh.context)
		{
			throw std::runtime_error("OptiXMesh: loadMesh() requires valid OptiX context");
		}

		optix::Context context = optix_mesh.context;

		Mesh mesh;
		MeshLoader loader(filename);
		loader.scanMesh(mesh);

		MeshBuffers buffers;
		setupMeshLoaderInputs(context, buffers, mesh);

		loader.loadMesh(mesh, load_xform.getData());

		translateMeshToOptiX(mesh, buffers, optix_mesh);

		unmap(buffers, mesh);
	}

} // namespace end