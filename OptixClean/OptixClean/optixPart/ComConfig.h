#pragma once
#include <string>

const std::string samplecu_file = "optixPart/sample.cu";
const std::string parallelogramcu_file = "optixPart/parallelogram.cu";
const std::string pathtracecu_file = "optixPart/pathtracing.cu";
const std::string pathpdfcu_file = "optixPart/pdfestimate.cu";
const std::string cornellmesh_file = "models/cornellbox.obj";
const std::string cube_mesh_file = "models/cube.obj";
const std::string myphonecu_file = "optixPart/myphong.cu";
const std::string trianglecu_file = "optixPart/triangle_mesh.cu";
const std::string pdfestimate_file = "optixPart/triangle_mesh.cu";

enum RAY_TYPE{ 
	color_ray_type = 0u, 
	color_shadow_ray_type, 
	sample_ray_type,
	sampleDirectlight_ray_type,
	pathtrace_ray_type,
	pathtrace_shadow_ray_type
};

const unsigned color_scene_entry = 0u;
const unsigned cam_sample_scene_entry = 1u;
const unsigned light_sample_scene_entry = 2u;
const unsigned directlight_sample_scene_entry = 3u;
const unsigned vpl_radiance_scene_entry = 4u;
const unsigned pathtrace_color_entry = 5u;
const unsigned pathtrace_pdf_entry = 6u;