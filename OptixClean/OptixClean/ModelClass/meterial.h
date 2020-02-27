#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;


#include "../Common/GLheader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include "Texture.h"
#include <SOIL/SOIL.h>

namespace ModelClass {

	class Meterial {
	public:
		Meterial() :
			specularColor(0, 0, 0),
			diffuseColor(0, 0, 0),
			ambientColor(0, 0, 0),
			shininess(0),
			hasSpecular(false),
			hasDiffuse(false),
			hasAmbient(false),
			hasShininess(false),

			hasNormal(false),
			hasHeight(false),
			hasLightmap(false),
			hasEmissive(false)
		{

		}
		string name;
		vec3 specularColor, diffuseColor, ambientColor;
		float shininess;
		string specularPath, diffusePath, ambientPath, shininessPath;
		Texture specularId, diffuseId, ambientId, shininessId;
		//0表示颜色，1表示贴图
		bool hasSpecular, hasDiffuse, hasAmbient, hasShininess;

		string normalPath, heightPath, lightmapPath, emissivePath;
		Texture normalId, heightId, lightmapId, emissiveId;
		bool hasNormal, hasHeight, hasLightmap, hasEmissive;
		//将材质根据shader绑定uniform
		unsigned int bindMeterial(GLuint shader, unsigned int startTexture = 0) {
			glUniform3fv(glGetUniformLocation(shader, "material.specularColor"), 1, &(specularColor)[0]);
			glUniform3fv(glGetUniformLocation(shader, "material.diffuseColor"), 1, &(diffuseColor)[0]);
			glUniform3fv(glGetUniformLocation(shader, "material.ambientColor"), 1, &(ambientColor)[0]);
			glUniform1f(glGetUniformLocation(shader, "material.shininess"), shininess);

			glUniform1i(glGetUniformLocation(shader, "material.hasSpecular"), hasSpecular ? 1 : 0);
			glUniform1i(glGetUniformLocation(shader, "material.hasDiffuse"), hasDiffuse ? 1 : 0);
			glUniform1i(glGetUniformLocation(shader, "material.hasAmbient"), hasAmbient ? 1 : 0);
			glUniform1i(glGetUniformLocation(shader, "material.hasShininess"), hasShininess ? 1 : 0);

			glUniform1i(glGetUniformLocation(shader, "material.hasNormal"), hasNormal ? 1 : 0);
			glUniform1i(glGetUniformLocation(shader, "material.hasHeight"), hasHeight ? 1 : 0);
			glUniform1i(glGetUniformLocation(shader, "material.hasLightmap"), hasLightmap ? 1 : 0);
			glUniform1i(glGetUniformLocation(shader, "material.hasEmissive"), hasEmissive ? 1 : 0);


			int i = startTexture;
			if (hasAmbient)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.ambientTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, ambientId.id);
				i++;
			}
			if (hasDiffuse)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.diffuseTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, diffuseId.id);
				i++;
			}
			if (hasSpecular)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.specularTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, specularId.id);
				i++;
			}
			if (hasShininess)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.shininessTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, shininessId.id);
				i++;
			}
			if (hasNormal)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.normalTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, normalId.id);
				i++;
			}
			if (hasHeight)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.heightTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, heightId.id);
				i++;
			}
			if (hasLightmap)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.lightMapTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, lightmapId.id);
				i++;
			}
			if (hasEmissive)
			{
				glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
				glUniform1i(glGetUniformLocation(shader, "material.emissiveTexture"), i);
				// And finally bind the texture
				glBindTexture(GL_TEXTURE_2D, emissiveId.id);
				i++;
			}
			return i;
		}
	};

}

