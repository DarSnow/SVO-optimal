#version 430 core
out vec4 FragColor;

#define NR_POINT_LIGHTS 1
//cell
uniform sampler2D depthMapFront;
uniform mat4 vplView;
uniform vec3 vplPos;
uniform vec3 vplCol;
uniform float constant;
uniform float linear;
uniform float quadratic;


const float SHADOW_EPSILON = 0.000005f;
//cell

in VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material 
{
	vec3 specularColor, diffuseColor, ambientColor;
	float shininess;
	int hasSpecular, hasDiffuse, hasAmbient, hasShininess;
	int hasNormal, hasHeight, hasLightmap, hasEmissive;
	sampler2D specularTexture, diffuseTexture, ambientTexture, shininessTexture;
    sampler2D normalTexture, heightTexture, lightmapTexture, emissiveTexture;
}; 
/* Note: because we now use a material struct again you want to change your
mesh class to bind all the textures using material.texture_diffuseN instead of
texture_diffuseN. */

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
uniform float near_plane;
uniform float far_plane;
uniform bool shadows;

//原代码里有light属性的bind，这里我暂且手动设置一下
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

//pointLights[0].position = vplPos;
//pointLights[0].constant = constant;
//pointLights[0].linear = linear;
//pointLights[0].quadratic = quadratic;
//pointLights[0].diffuse = vplCol;

float ShadowCalculation(vec3 fragPos)
{
	//cell
	vec3 vPosDP = (vec4(fs_in.FragPos, 1) * vplView).xyz;
	vPosDP.x += vplPos.x;
	vPosDP.y += vplPos.y;
	vPosDP.z += vplPos.z;

	vec3 fragToLight = fragPos - vplPos;
	float fLength = length(fragToLight);
	//这里/fLength可能有问题
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
		fDPDepth = texture(depthMapFront, vTexFront.xy).r;
	}
	float shadow = fSceneDepth -  0.05 > fDPDepth ? 1.0 : 0.0;  

    //FragColor = vec4(vec3(fDPDepth), 1.0);    
	//FragColor = vec4(test, 1.0); 

    return shadow;
}

//使用lambert模型，下面函数仅计算diffuse
vec3 CalcPointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
	vec3 diffuse = vec3(0,0,0);

	if(mat.hasDiffuse==1)
		diffuse = light.diffuse * diff * vec3(texture(mat.diffuseTexture, fs_in.TexCoords));
	else
		//两次diff?
		diffuse = light.diffuse * diff * mat.diffuseColor*diff;

	diffuse *= attenuation;
	return diffuse;
}


void main()
{           
    
	vec3 result = vec3(0,0,0);
	//viewDir是算高光的时候用的
    //vec3 viewDir = normalize(viewPos - fs_in.FragPos );
	vec3 norm = vec3(0,0,0);
	if(material.hasNormal==1)
	{
		norm = texture(material.normalTexture, fs_in.TexCoords).rgb;
		norm = normalize(norm * 2.0 - 1.0);  
	}
	else
		norm = normalize(fs_in.Normal);

	vec3 lightDir = normalize(vplPos - fs_in.FragPos);
    float diff = max(dot(lightDir, norm), 0.0);

	vec3 color = material.diffuseColor * diff;
	vec3 lightColor = vplCol;
	// ambient
    vec3 ambient = 0.3 * color;
	// diffuse    
	float constant = 0.5f;
	float linear = 0.3f;
	float quadratic = 0.2f;
	float distance = length(vplPos - fs_in.FragPos);
	float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
    vec3 diffuse = diff * lightColor * attenuation;
	// calculate shadow
	float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;  
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color;
	FragColor = vec4(lighting, 1.0);

//vec3 ambient;
//
//float diff = max(dot(norm, viewDir), 0.0)
//
//if (material.hasDiffuse == 1)
//ambient = 0.3 * vec3(texture(material.diffuseTexture, fs_in.TexCoords));
//else
//ambient = 0.3 * vec3(material.diffuseColor);
//
//float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;
////FragColor = vec4(ambient, 1.0f); 
////shadow取1时，判定为阴影 
//if (shadow == 1)
//{
//	FragColor = vec4(ambient, 1.0f);
//}
//else
//{
//	if (material.hasNormal == 1)
//	{
//		norm = texture(material.normalTexture, fs_in.TexCoords).rgb;
//		norm = normalize(norm * 2.0 - 1.0);
//	}
//	else
//		norm = normalize(fs_in.Normal);
//
//	for (int i = 0; i < NR_POINT_LIGHTS; i++)
//		result += CalcPointLight(pointLights[i], material, norm, fs_in.FragPos, viewDir);
//
//	FragColor = vec4(ambient + result, 1.0f);
//}

	
}