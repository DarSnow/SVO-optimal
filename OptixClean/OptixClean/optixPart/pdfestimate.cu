#pragma once
#include <optixu/optixu_math_namespace.h>
#include "commonStructs.h"
#include "random.h"
using namespace optix;

struct PerRayData_pathtrace_shadow
{
    bool inShadow;
};


rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(uint,          launch_index, rtLaunchIndex, );

rtDeclareVariable(unsigned int,  frame_number, , );
rtDeclareVariable(unsigned int,  pathtrace_shadow_ray_type, , );
rtDeclareVariable(unsigned int,  sensor_size, , );

rtBuffer<opVPL>					vpl_buffer;
rtBuffer<float3>				sensor_pos_buffer;
rtBuffer<float3>				sensor_nor_buffer;
rtBuffer<float>					contri_buffer;

RT_PROGRAM void estimate_pdf_entry()
{
	opVPL vpl = vpl_buffer[launch_index];
	float3 sensor = sensor_pos_buffer[launch_index];
	float3 sensor_nor = sensor_nor_buffer[launch_index];

	float contribution = 0;
	for(int i=0;i<sensor_size;++i){
		PerRayData_pathtrace_shadow shadow_prd;
        shadow_prd.inShadow = false;
		const float3 L = normalize( sensor - vpl.pos);
		const float  Ldist = length( sensor - vpl.pos);
        Ray shadow_ray = make_Ray( vpl.pos, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist - scene_epsilon );
        rtTrace(top_object, shadow_ray, shadow_prd);

        if(!shadow_prd.inShadow && dot(sensor_nor, -L) > 0)
        {
            contribution += dot( vpl.normal, L ) * dot(sensor_nor, -L) / ( Ldist * Ldist);
        }
	}
	contri_buffer[launch_index] = contribution / sensor_size;
}

RT_PROGRAM void exception(){
	
}