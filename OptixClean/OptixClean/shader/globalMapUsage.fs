//顶光源以及vpl的渲染
#version 430 core
out vec4 FragColor;

#define maxvplNUM 32

//MAIN_LIGHT_MODEL主光源的SM模式
//0表示使用双面PSM
//1表示使用六面cubeSM
const int MAIN_LIGHT_MODEL = 1;
float SHADOW_EPSILON = 0.005f;
uniform sampler2D depthMapFront;
uniform sampler2D depthMapBack;
uniform samplerCube depthMap;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightNor;
uniform mat4 lightView;

in VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform vec3 viewPos;
uniform float near_plane;
uniform float far_plane;

uniform bool shadows;

struct Material 
{
	vec3 specularColor, diffuseColor, ambientColor;
	float shininess;
	int hasSpecular, hasDiffuse, hasAmbient, hasShininess;
	int hasNormal, hasHeight, hasLightmap, hasEmissive;
	sampler2D specularTexture, diffuseTexture, ambientTexture, shininessTexture;
    sampler2D normalTexture, heightTexture, lightmapTexture, emissiveTexture;
}; 

struct PointLight 
{
    vec3 position;
    mat4 view;
	vec3 normal;

    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//vplNUM个vpl属性，以及对应的vplDepthMap
uniform int vplNUM;
uniform PointLight pointLights[maxvplNUM];
uniform sampler2D vplDepthMap[maxvplNUM];
//这个材质material是场景的材质，不是光源的材质
uniform Material material;

float ShadowCalculation1(vec3 fragPos, sampler2D depthMap, vec3 vplPos, mat4 vplView);
float ShadowCalculation2(vec3 fragPos, sampler2D tdepthMapFront, sampler2D tdepthMapBack);
float CubeShadowCalculation(vec3 fragPos);

void main()
{          
	vec3 result = vec3(0,0,0);
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
	// 漫反射平面上点光源向外发射能量的分布，符合cos，是点光源(出射点)法线与出射方向的夹角
	float lightEn = max(dot(lightDir, -lightNor), 0.0);
	//float vplCos = max(dot(lightDir, normal), 0.0)
	vec3 color = material.diffuseColor ;
	
	SHADOW_EPSILON = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	//先计算主光源即顶部光源的贡献
	// ambient
    vec3 ambient = 0.5 * color;
	//FragColor = vec4(material.diffuseColor, 1.0);
	//return;
	// diffuse    
	float constant = 0.5f;
	float linear = 0.3f;
	float quadratic = 0.2f;
	float distance = length(lightPos - fs_in.FragPos);
	//float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
	float attenuation = 1.0f / distance;
    vec3 diffuse = diff * lightCol * attenuation;
	// calculate shadow
	float shadow = 0.0f;
	if(MAIN_LIGHT_MODEL == 0)
	{
		//MAIN_LIGHT_MODEL为0使用双面PSM
		shadow = shadows ? ShadowCalculation2(fs_in.FragPos, depthMapFront, depthMapBack) : 0.0; 
	}
	else
	{
		//MAIN_LIGHT_MODEL为1使用六面CSM
		shadow = shadows ? CubeShadowCalculation(fs_in.FragPos) : 0.0;
	}  
	if(shadow == 1.0f)
	{
		result = 0.8 * ambient * color;
	}
	else
	{
		//result = (ambient + (1.0 - shadow) * (diffuse) * lightCos) * color;
		//result = ambient + (1.0 - shadow) * diffuse * color;
		result = (ambient + (1.0 - shadow) * (diffuse) * (lightEn)) * color;
	}
	
	//再计算vpl的贡献
	for(int i = 0; i < vplNUM; i++)
	{
		vec3 vplDir = normalize(pointLights[i].position - fs_in.FragPos);
		// 针对入射点法线与入射方向的夹角
		float vplDiff = max(dot(vplDir, normal), 0.0);
		// 漫反射平面上vpl向外发射能量的分布，符合cos，是vpl(出射点)法线与出射方向的夹角
		float vplEn = max(dot(vplDir, pointLights[i].normal), 0.0);
		//float vplEn = 1.0f;
		// ambient
		vec3 vplAmbient = vec3(0,0,0);
		// diffuse 
		float vplDistance = length(pointLights[i].position - fs_in.FragPos);
		//if(vplDistance < 0.5)
		//	continue;
		float vplAttenuation = 1.0f / (pointLights[i].constant + pointLights[i].linear * vplDistance + pointLights[i].quadratic * (vplDistance * vplDistance));
		vec3 vplDiffuse = vplDiff * pointLights[i].diffuse * vplAttenuation;
		// calculate shadow
		// float vplShadow = 0.0f;
		float vplShadow = shadows ? ShadowCalculation1(fs_in.FragPos, vplDepthMap[i], pointLights[i].position, pointLights[i].view) : 0.0;  
		
		result += ((vplAmbient + (1.0 - vplShadow) * (vplDiffuse) * (vplEn)) * color)/10;	
	}

	FragColor = vec4(result, 1.0);
}

float CubeShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  SHADOW_EPSILON > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}

float ShadowCalculation2(vec3 fragPos, sampler2D tdepthMapFront, sampler2D tdepthMapBack)
{
	vec3 vPosDP = (vec4(fs_in.FragPos, 1) * lightView).xyz;
	vec3 fragToLight = fragPos - lightPos;
	float fLength = length(fragToLight);
	//是这样的吗？
	vPosDP.x += lightPos.x;
	vPosDP.y += lightPos.y;
	vPosDP.z += lightPos.z;
	vPosDP /= fLength;

    float fDPDepth;
    float fSceneDepth;
	vec3 test = vPosDP.xyz;
	if(vPosDP.z >= 0.0f)
    {
		vec2 vTexFront;
        vTexFront.x =  ((vPosDP.x /  (1.0f + vPosDP.z)) * 0.5f + 0.5f); 
        vTexFront.y =  ((vPosDP.y /  (1.0f + vPosDP.z)) * 0.5f + 0.5f);     

        //fSceneDepth = (fLength - near_plane) / (far_plane - near_plane);

		fSceneDepth = fLength / far_plane;
		fDPDepth = texture(tdepthMapFront, vTexFront.xy).r;
	}
	else
	{     
        vec2 vTexBack;
        vTexBack.x =  ((vPosDP.x /  (1.0f - vPosDP.z)) * 0.5f + 0.5f); 
        vTexBack.y =  ((vPosDP.y /  (1.0f - vPosDP.z)) * 0.5f + 0.5f); 
		fSceneDepth = fLength / far_plane;
		fDPDepth = texture(tdepthMapBack, vTexBack.xy).r;
	}
	
	float shadow = fSceneDepth -  SHADOW_EPSILON > fDPDepth ? 1.0 : 0.0;  
    return shadow;
}

float ShadowCalculation1(vec3 fragPos, sampler2D depthMap, vec3 vplPos, mat4 vplView)
{
	//这里不应该用lightView
	//vec3 vPosDP = (vec4(fs_in.FragPos, 1) * lightView).xyz;
	vec3 vPosDP = (vec4(fragPos, 1) * vplView).xyz;
	vec3 fragToLight = fragPos - vplPos;
	float fLength = length(fragToLight);
	//是这样的吗？
	vPosDP.x += vplPos.x;
	vPosDP.y += vplPos.y;
	vPosDP.z += vplPos.z;
	vPosDP /= fLength;

    float fDPDepth;
    float fSceneDepth;
	vec3 test = vPosDP.xyz;
	if(vPosDP.z >= 0.0f)
    {
		vec2 vTexFront;
        vTexFront.x =  ((vPosDP.x /  (1.0f + vPosDP.z)) * 0.5f + 0.5f); 
        vTexFront.y =  ((vPosDP.y /  (1.0f + vPosDP.z)) * 0.5f + 0.5f);     
		
		fSceneDepth = fLength / far_plane;
		fDPDepth = texture(depthMap, vTexFront.xy).r;
	}
	else
	{     
		fSceneDepth = 1.0;
		fDPDepth = 0.0;
	}
	
	float shadow = fSceneDepth -  SHADOW_EPSILON > fDPDepth ? 1.0 : 0.0;  
    return shadow;
}