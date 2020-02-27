		//单面，仅有depthMapUpFBO的map

		//#version 430 core
		//	out vec4 FragColor;

		////cell
		////uniform sampler2D depthMapFront;
		////uniform sampler2D depthMapBack;
		//uniform sampler2D depthMapUp;
		//uniform vec3 lightPos;

		//uniform mat4 lightView;

		////varying vec3 vPosDP;

		//const float SHADOW_EPSILON = 0.000005f;
		////cell


		//in VS_OUT
		//{
		//	vec3 FragPos;
		//vec3 Normal;
		//vec2 TexCoords;
		//} fs_in;

		//uniform sampler2D diffuseTexture;


		//uniform samplerCube depthMap;

		////uniform vec3 lightPos;
		//uniform vec3 viewPos;
		//uniform float near_plane;
		//uniform float far_plane;
		//uniform bool shadows;

		//float getDepthFromARGB32(vec4 value)
		//{
		//	const vec4 bitSh = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
		//	return(dot(value, bitSh));
		//}


		//float ShadowCalculation(vec3 fragPos)
		//{
		//	//cell
		//	vec3 vPosDP = (vec4(fs_in.FragPos, 1) * lightView).xyz;

		//	//尝试使用fragToLight
		//	vec3 fragToLight = fragPos - lightPos;

		//	float fLength = length(fragToLight);

		//	vPosDP /= fLength;

		//	float fDPDepth;
		//	float fSceneDepth;

		//	if (vPosDP.z >= 0.0f)
		//	{
		//		vec2 vTexFront;
		//		vTexFront.x = ((vPosDP.x / (1.0f + vPosDP.z)) * 0.5f + 0.5f);
		//		vTexFront.y = ((vPosDP.y / (1.0f + vPosDP.z)) * 0.5f + 0.5f);


		//		//fSceneDepth = (fLength - near_plane) / (far_plane - near_plane);

		//		fSceneDepth = fLength / far_plane;

		//		fDPDepth = texture(depthMapUp, vTexFront.xy).r;

		//		//fDPDepth = getDepthFromARGB32(texture(depthMapUp, vTexFront.xy));

		//	}
		//	else
		//	{
		//		//// for the back the z has to be inverted        
		//		//vec2 vTexBack;
		//		//vTexBack.x = ((vPosDP.x / (1.0f - vPosDP.z)) * 0.5f + 0.5f);
		//		//vTexBack.y = ((vPosDP.y / (1.0f - vPosDP.z)) * 0.5f + 0.5f);

		//		////fSceneDepth = (fLength - near_plane) / (far_plane - near_plane);

		//		//fSceneDepth = fLength / far_plane;

		//		//fDPDepth = texture(depthMapBack, vTexBack.xy).r;

		//		////fDPDepth = getDepthFromARGB32(texture(depthMapBack, vTexBack.xy)); 

		//	}

		//	float shadow = fSceneDepth - 0.05 > fDPDepth ? 1.0 : 0.0;

		//	//cell

		//	// get vector between fragment position and light position
		//	//vec3 fragToLight = fragPos - lightPos;
		//	// ise the fragment to light vector to sample from the depth map    
		//	float closestDepth = texture(depthMap, fragToLight).r;
		//	// it is currently in linear range between [0,1], let's re-transform it back to original depth value
		//	closestDepth *= far_plane;
		//	// now get current linear depth as the length between the fragment and light position
		//	float currentDepth = length(fragToLight);
		//	// test for shadows
		//	float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
		//					   // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
		//					   // display closestDepth as debug (to visualize depth cubemap)

		//					   //FragColor = vec4(vec3(fDPDepth), 1.0);    

		//	return shadow;
		//}

		//void main()
		//{
		//	vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
		//	vec3 normal = normalize(fs_in.Normal);
		//	vec3 lightColor = vec3(0.3);
		//	// ambient
		//	vec3 ambient = 0.3 * color;
		//	// diffuse
		//	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
		//	float diff = max(dot(lightDir, normal), 0.0);
		//	vec3 diffuse = diff * lightColor;
		//	// specular
		//	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
		//	vec3 reflectDir = reflect(-lightDir, normal);
		//	float spec = 0.0;
		//	vec3 halfwayDir = normalize(lightDir + viewDir);
		//	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
		//	vec3 specular = spec * lightColor;
		//	// calculate shadow
		//	float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;
		//	//shadow取1时，判定为阴影              
		//	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

		//	FragColor = vec4(lighting, 1.0);
		//}


//双面的map
#version 430 core
out vec4 FragColor;

//cell
uniform sampler2D depthMapFront;
uniform sampler2D depthMapBack;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform mat4 lightView;

//varying vec3 vPosDP;

const float SHADOW_EPSILON = 0.000005f;
//cell


in VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;


uniform samplerCube depthMap;

uniform vec3 viewPos;
uniform float near_plane;
uniform float far_plane;
uniform bool shadows;

struct Material {
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
uniform Material material;

float getDepthFromARGB32(vec4 value)
{
    const vec4 bitSh = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    return(dot(value, bitSh));
}


float ShadowCalculation(vec3 fragPos)
{



	//cell
	vec3 vPosDP = (vec4(fs_in.FragPos, 1) * lightView).xyz;

	//尝试使用fragToLight
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

		fDPDepth = texture(depthMapFront, vTexFront.xy).r;

		//fDPDepth = getDepthFromARGB32(texture(depthMapFront, vTexFront.xy));

	}
	else
	{
		// for the back the z has to be inverted        
        vec2 vTexBack;
        vTexBack.x =  ((vPosDP.x /  (1.0f - vPosDP.z)) * 0.5f + 0.5f); 
        vTexBack.y =  ((vPosDP.y /  (1.0f - vPosDP.z)) * 0.5f + 0.5f); 

        //fSceneDepth = (fLength - near_plane) / (far_plane - near_plane);

		fSceneDepth = fLength / far_plane;

		fDPDepth = texture(depthMapBack, vTexBack.xy).r;

		//fDPDepth = getDepthFromARGB32(texture(depthMapBack, vTexBack.xy)); 

	}

	float shadow = fSceneDepth -  0.05 > fDPDepth ? 1.0 : 0.0;  

	//cell

    // get vector between fragment position and light position
    //vec3 fragToLight = fragPos - lightPos;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)

    //FragColor = vec4(vec3(fDPDepth), 1.0);    
	//FragColor = vec4(test, 1.0); 

    return shadow;
}

void main()
{           
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);

    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 color = material.diffuseColor*diff;
    
    //vec3 lightColor = vec3(0.3);
	vec3 lightColor = lightCol;
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse    
	float constant = 0.5f;
	float linear = 0.3f;
	float quadratic = 0.2f;
	float distance = length(lightPos - fs_in.FragPos);
	float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
    vec3 diffuse = diff * lightColor * attenuation;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;        
	//shadow取1时，判定为阴影              
    //vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color;   
    
    FragColor = vec4(lighting, 1.0);
}