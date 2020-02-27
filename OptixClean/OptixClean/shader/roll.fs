// Fragment shader:
// ================
#version 430 core

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

struct PointLight 
{
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 1

in vec3 fragPosition;
in vec3 Normal;
in vec2 mTexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;


// Function prototypes
vec3 CalcPointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    vec3 result = vec3(0,0,0);
    vec3 viewDir = normalize(viewPos - fragPosition );
	vec3 norm = vec3(0,0,0);
	if(material.hasNormal==1)
	{
		norm = texture(material.normalTexture, mTexCoords).rgb;
		norm = normalize(norm * 2.0 - 1.0);  
	}
	else
		norm = normalize(Normal);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], material, norm, fragPosition, viewDir);
    if(material.hasLightmap==1)
		color *= texture(material.lightmapTexture, mTexCoords).r;
	
	color = vec4(result, 1.0f); 
	//color = vec4(fragPosition, 1.0f); 

}


// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = 0.0f ;
	if(dot(viewDir, reflectDir)>0)
		spec = pow(dot(viewDir, reflectDir), max(mat.shininess,32));
	else
		spec = 0;
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
	// Combine results
    vec3 ambient = vec3(0,0,0),diffuse = vec3(0,0,0) ,specular  = vec3(0,0,0);
	if(mat.hasAmbient==1)
		ambient = light.ambient * vec3(texture(mat.ambientTexture, mTexCoords));
	else
		ambient = light.ambient * mat.ambientColor;
	if(mat.hasDiffuse==1)
		diffuse = light.diffuse * diff * vec3(texture(mat.diffuseTexture, mTexCoords));
	else
		//diffuse = light.diffuse * diff * mat.diffuseColor*diff;
		diffuse = light.diffuse * diff * mat.diffuseColor;
	if(mat.hasSpecular==1)
		specular= light.specular * spec * spec * vec3(texture(mat.specularTexture, mTexCoords));
	else
		specular = light.specular * spec * mat.specularColor;

    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
