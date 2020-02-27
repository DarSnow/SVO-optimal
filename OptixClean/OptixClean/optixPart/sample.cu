#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_world.h>
#include "helpers.h"
#include "commonStructs.h"
#include "random.h"

using namespace optix;

struct PerRayData
{
  float3 pos;
  float3 nor;
  float3 clr;
  float importance;
  int depth;
  unsigned int seed;
};

rtDeclareVariable(unsigned int,      sample_ray_type, , );
rtDeclareVariable(unsigned int,      sampleDirectlight_ray_type, , );
rtDeclareVariable(float,             scene_epsilon, , );
rtDeclareVariable(rtObject,          top_object, , );

rtDeclareVariable(PerRayData, prd, rtPayload, );

rtDeclareVariable(float3,       Ka, , );
rtDeclareVariable(float3,       Kd, , );
rtDeclareVariable(float3,       Ks, , );
rtDeclareVariable(float3,       Kr, , );
rtDeclareVariable(float,        phong_exp, , );


rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(float3,		 bg_color, , );
rtBuffer<float4>                 sample_pos_buffer;
rtBuffer<float4>                 sample_nor_buffer;
rtBuffer<float4>                 sample_clr_buffer;

rtDeclareVariable(unsigned int,  frame_number, , );

rtDeclareVariable(uint, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint, launch_dim,   rtLaunchDim, );

// sample cam ray gen program
RT_PROGRAM void pinhole_camera_samCam()
{
  unsigned int seed = tea<16>(launch_index, frame_number);
  float2 d = make_float2(rnd(seed), rnd(seed));

  float3 ray_origin = eye;
  float3 ray_direction = normalize(d.x*U + d.y*V + W);
  
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, sample_ray_type, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData newprd;
  newprd.importance = 1.f;
  newprd.depth = 0;
  newprd.seed = seed;

  rtTrace(top_object, ray, newprd);

  sample_pos_buffer[launch_index] = make_float4( newprd.pos.x, newprd.pos.y, newprd.pos.z, 1);
  sample_nor_buffer[launch_index] = make_float4( newprd.nor.x, newprd.nor.y, newprd.nor.z, 1);
  sample_clr_buffer[launch_index] = make_float4( newprd.clr.x, newprd.clr.y, newprd.clr.z, 1);
}


rtDeclareVariable(float3,     geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3,     shading_normal,   attribute shading_normal, );
rtDeclareVariable(optix::Ray, ray,              rtCurrentRay, );
rtDeclareVariable(float,      t_hit,            rtIntersectionDistance, );

// used to handle closest hit of camera sample and one bounce light sample
RT_PROGRAM void closest_hit_sample()
{
	float3 world_geo_normal = normalize(rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal));
	float3 world_shade_normal = normalize(rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(world_shade_normal, -ray.direction, world_geo_normal);

	float3 hit_point = ray.origin + t_hit * ray.direction;
	
	if(prd.depth<1){
		prd.depth += 1;
		float z1=rnd(prd.seed);
		float z2=rnd(prd.seed);
		float3 p;
		cosine_sample_hemisphere(z1, z2, p);
		optix::Onb onb( ffnormal );
		onb.inverse_transform( p );
		optix::Ray refl_ray = optix::make_Ray( hit_point, p, sample_ray_type, scene_epsilon, RT_DEFAULT_MAX );
		rtTrace(top_object, refl_ray, prd);
	}
	else{
		prd.pos = hit_point;
		prd.nor = ffnormal;
		prd.clr = Kd;
	}
}

__device__ int getRandSign(unsigned int& seed)
{
	return (rnd(seed)<0.5f)?1:-1;
}

rtBuffer<BasicLight>                 lights;

// sample light ray gen program
RT_PROGRAM void samdirect_light_raygen()
{
  unsigned int seed = tea<16>(launch_index, frame_number);

  BasicLight light = lights[0];
  float3 ray_origin = light.pos;
  
  float z1=rnd(seed);
  float z2=rnd(seed);
  float3 ray_direction;
  cosine_sample_hemisphere(z1, z2, ray_direction);
  optix::Onb onb( light.normal );
  onb.inverse_transform( ray_direction );
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, sampleDirectlight_ray_type, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData newprd;
  newprd.importance = 1.f;
  newprd.depth = 0;
  newprd.seed = seed;

  rtTrace(top_object, ray, newprd);

  sample_pos_buffer[launch_index] = make_float4( newprd.pos.x, newprd.pos.y, newprd.pos.z, 1);
  sample_nor_buffer[launch_index] = make_float4( newprd.nor.x, newprd.nor.y, newprd.nor.z, 1);
  sample_clr_buffer[launch_index] = make_float4( newprd.clr.x, newprd.clr.y, newprd.clr.z, 1);
}

RT_PROGRAM void samlight_raygen()
{
  unsigned int seed = tea<16>(launch_index, frame_number);

  BasicLight light = lights[0];
  float3 ray_origin = light.pos;
  
  float z1=rnd(seed);
  float z2=rnd(seed);
  float3 ray_direction;
  cosine_sample_hemisphere(z1, z2, ray_direction);
  optix::Onb onb( light.normal );
  onb.inverse_transform( ray_direction );
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, sample_ray_type, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData newprd;
  newprd.importance = 1.f;
  newprd.depth = 0;
  newprd.seed = seed;

  rtTrace(top_object, ray, newprd);

  sample_pos_buffer[launch_index] = make_float4( newprd.pos.x, newprd.pos.y, newprd.pos.z, 1);
  sample_nor_buffer[launch_index] = make_float4( newprd.nor.x, newprd.nor.y, newprd.nor.z, 1);
  sample_clr_buffer[launch_index] = make_float4( newprd.clr.x, newprd.clr.y, newprd.clr.z, 1);
}

// sample point directly from light (Dirlight)
RT_PROGRAM void closest_hit_samplelight()
{
	float3 world_geo_normal = normalize(rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal));
	float3 world_shade_normal = normalize(rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(world_shade_normal, -ray.direction, world_geo_normal);
	float3 hit_point = ray.origin + t_hit * ray.direction;
	prd.depth += 1;
	prd.pos = hit_point;
	prd.nor = ffnormal;
	prd.clr = Kd;
}


// sample miss program
RT_PROGRAM void miss()
{
  prd.pos = bg_color;
  prd.nor = bg_color;
  prd.clr = bg_color;
}

// sample ray exception program
RT_PROGRAM void exception()
{
  const unsigned int code = rtGetExceptionCode();
  rtPrintf( "Caught exception 0x%X at launch index (%d)\n", code, launch_index );
  sample_pos_buffer[launch_index] = make_float4(bad_color.x, bad_color.y, bad_color.z, 1);
  sample_nor_buffer[launch_index] = make_float4(bad_color.x, bad_color.y, bad_color.z, 1);
  sample_clr_buffer[launch_index] = make_float4(bad_color.x, bad_color.y, bad_color.z, 1);
}