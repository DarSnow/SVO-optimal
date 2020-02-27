#version 430
#extension GL_ARB_shader_image_load_store : require

in GeometryOut
{
    vec3 wsPosition;
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    flat vec4 triangleAABB;
} In;

layout (location = 0) out vec4 fragColor;
layout (pixel_center_integer) in vec4 gl_FragCoord;

layout(binding = 0, rgb10_a2ui) uniform uimageBuffer voxelPos;
layout(binding = 1, rgba8) uniform volatile coherent imageBuffer voxelAlbedo;
layout(binding = 2, rgba8) uniform volatile coherent imageBuffer voxelNormal;
layout(binding = 3, rgba8) uniform volatile coherent imageBuffer voxelEmission;

//atomic counter 
layout(binding = 0, offset = 0) uniform atomic_uint u_voxelFragCount;

uniform struct Material
{
	vec3 specularColor, diffuseColor, ambientColor;
	float shininess;
	int hasSpecular, hasDiffuse, hasAmbient, hasShininess;
	int hasNormal, hasHeight, hasLightmap, hasEmissive;
	sampler2D specularTexture, diffuseTexture, ambientTexture, shininessTexture;
    sampler2D normalTexture, heightTexture, lightmapTexture, emissiveTexture;
} material;

uniform int isStore;
uniform uint volumeDimension;

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}

void main()
{
    if( In.position.x < In.triangleAABB.x || In.position.y < In.triangleAABB.y || 
		In.position.x > In.triangleAABB.z || In.position.y > In.triangleAABB.w )
	{
		discard;
	}
    // writing coords position
    uvec3 position = uvec3(In.wsPosition);
    // fragment albedo
	vec4 albedo = vec4(1.0f);
    if(material.hasDiffuse == 1)
		albedo = texture(material.diffuseTexture, In.texCoord);

	uint idx = atomicCounterIncrement(u_voxelFragCount);
    // alpha cutoff
    if(isStore==1)
    {
        // albedo is in srgb space, bring back to linear
        albedo.rgb = material.diffuseColor * albedo.rgb;
        // premultiplied alpha
        albedo.rgb *= albedo.a;
        albedo.a = 1.0f;
        // emission value
        vec4 emissive = vec4(0);
		if(material.hasEmissive == 1)
		     emissive = texture(material.emissiveTexture, In.texCoord);
        emissive.a = 1.0f;
        // bring normal to 0-1 range
        vec4 normal = vec4(EncodeNormal(normalize(In.normal)), 1.0f);
		//albedo = vec4(0.5, 0,0, 1);
		imageStore(voxelPos, int(idx), uvec4(position,0));
		memoryBarrier();
		imageStore(voxelAlbedo, int(idx), albedo);
		memoryBarrier();
		imageStore(voxelNormal, int(idx), normal);
		memoryBarrier();
		imageStore(voxelEmission, int(idx), emissive);
    }
}