#version 430 core

out vec4 FragColor;
in vec2 TexCoords;

//MAIN_LIGHT_MODEL主光源的SM模式
//0表示使用双面PSM
//1表示使用六面cubeSM
const int MAIN_LIGHT_MODEL = 1;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform sampler2D depthMapFront;
uniform sampler2D depthMapBack;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform mat4 lightView;

uniform vec3 viewPos;
uniform float near_plane;
uniform float far_plane;
uniform bool shadows;


struct PointLight 
{
	mat4 view;
    vec3 position;	    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

const int vplNUM = 30;
const float SHADOW_EPSILON = 0.005f;

uniform PointLight pointLights[vplNUM];
uniform sampler2D vplDepthMap[vplNUM];

float CubeShadowCalculation(vec3 fragPos)
{

    vec3 fragToLight = fragPos - lightPos;
    float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= far_plane;
    float currentDepth = length(fragToLight);
    float shadow = currentDepth -  SHADOW_EPSILON > closestDepth ? 1.0 : 0.0; 
        
    return shadow;
}

float ShadowCalculation2(vec3 fragPos, sampler2D tdepthMapFront, sampler2D tdepthMapBack)
{
	//vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 vPosDP = (vec4(fragPos, 1) * lightView).xyz;
	vec3 fragToLight = fragPos - lightPos;
	float fLength = length(fragToLight);

	vPosDP.x += lightPos.x;
	vPosDP.y += lightPos.y;
	vPosDP.z += lightPos.z;
	vPosDP /= fLength;

    float fDPDepth;
    float fSceneDepth;
	//vec3 test = vPosDP.xyz;
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


void main()
{          
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;

	vec3 result = vec3(0,0,0);
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
	vec3 color = Diffuse  * diff;
	
	//先计算主光源即顶部光源的贡献
	// ambient
    vec3 ambient = 0.3 * color;
	// diffuse    
	float constant = 0.5f;
	float linear = 0.3f;
	float quadratic = 0.2f;
	float distance = length(lightPos - FragPos);
	//float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
	float attenuation = 1.0f / distance;
    vec3 diffuse = diff * lightCol * attenuation;
	// calculate shadow
	float shadow = 0.0f;
	if(MAIN_LIGHT_MODEL == 0)
	{
		//MAIN_LIGHT_MODEL为0使用双面PSM
		shadow = shadows ? ShadowCalculation2(FragPos, depthMapFront, depthMapBack) : 0.0; 
	}
	else
	{
		//MAIN_LIGHT_MODEL为1使用六面CSM
		shadow = shadows ? CubeShadowCalculation(FragPos) : 0.0;
	}  
	result = (ambient + (1.0 - shadow) * (diffuse)) * color;
	
	//再计算vpl的贡献
	for(int i = 0; i < vplNUM; i++)
	{
		vec3 vplDir = normalize(pointLights[i].position - FragPos);
		float vplDiff = max(dot(vplDir, normal), 0.0);
		// ambient
		vec3 vplAmbient = vec3(0,0,0);
		// diffuse 
		float vplDistance = length(pointLights[i].position - FragPos);			
		float vplAttenuation = 1.0f / (pointLights[i].constant + pointLights[i].linear * vplDistance + pointLights[i].quadratic * (vplDistance * vplDistance));
		vec3 vplDiffuse = vplDiff * pointLights[i].diffuse * vplAttenuation;
		// calculate shadow
		// float vplShadow = 0.0f;
		float vplShadow = shadows ? ShadowCalculation1(FragPos, vplDepthMap[i], pointLights[i].position, pointLights[i].view) : 0.0;  
		result += ((vplAmbient + (1.0 - vplShadow) * (vplDiffuse)) * color) / 10;		
	}
	FragColor = vec4(result, 1.0);
}
