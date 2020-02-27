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

#include <SOIL/SOIL.h>

namespace ModelClass {
	class Texture {
	public:
		string path;
		long m_width, m_height;
		GLuint id;
		//空贴图
		Texture() :id(0) {}
		//从文件读纹理，有mipmap
		Texture(string _path) :path(_path)
		{
			//Generate texture ID and load texture data 
			string filename = path;
			GLuint textureID;
			glGenTextures(1, &textureID);
			int width, height;
			unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
			if (image == NULL) {
				cout << "texture load failed.can't open " << filename << endl;
				return;
			}
			m_width = width;
			m_height = height;
			// Assign texture to ID
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);

			// Parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			SOIL_free_image_data(image);
			id = textureID;
		}

		~Texture() {
			
		}
	};

	class Texture3D {
	public:
		string path;
		long m_width, m_height;
		GLuint id;
		//空贴图
		Texture3D() :id(0) {}
		// texture 3d
		Texture3D(int dim, GLenum informat, GLenum format)
		{
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_3D, id);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage3D(GL_TEXTURE_3D, 0, informat, dim, dim, dim, 0, format, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_3D, 0);
			GLenum err = glGetError();
			if(err)
				cout << glewGetErrorString(err) << " " << err << endl;
		}

		~Texture3D() {

		}
	};
	class TextureBuffer
	{
	public:
		GLuint texId, tbo;
		int size_;
		GLenum format_;
		//默认构造
		TextureBuffer() :texId(0), tbo(0) {}
		TextureBuffer(int size, GLenum format)
		{
			while (glGetError());
			size_ = size;
			format_ = format;
			/*glCreateBuffers(1, &tbo);
			glNamedBufferStorage(tbo, size, nullptr, 0);
			glCreateTextures(1, GL_TEXTURE_BUFFER, &texId);
			glTextureBuffer(texId, format, tbo);*/
			unsigned char *temp = new unsigned char[size];
			memset(temp, 0, size);
			GLenum err;
			glGenBuffers(1, &tbo);
			glBindBuffer(GL_TEXTURE_BUFFER, tbo);
			glBufferData(GL_TEXTURE_BUFFER, size, temp, GL_STATIC_DRAW);
			err = glGetError();

			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_BUFFER, texId);
			glTexBuffer(GL_TEXTURE_BUFFER, format, tbo);
			glBindBuffer(GL_TEXTURE_BUFFER, 0);

			err = glGetError();
			if (err > 0)
				cout << "Error TextureBuffer():" << glewGetErrorString(err) << " " << err << endl;
		}
		~TextureBuffer() {
		/*	glDeleteTextures(1, &texId);
			glDeleteBuffers(1, &tbo);*/
		};
	};
	class AtomicBuffer
	{
	public:
		GLuint bufId;
		AtomicBuffer() :bufId(0) {};
		AtomicBuffer(int num)
		{
			while (glGetError());
			//GLuint initVal = 0;
			glGenBuffers(1, &bufId);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bufId);
			glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_DRAW);
			ResetCounter(0);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, bufId);
			GLenum err = glGetError();
			if (err > 0)
				cout << "Error AtomicBuffer():" << glewGetErrorString(err) << " " << err << endl;
		}
		GLuint GetCounter()
		{
			while (glGetError());
			GLuint *counts;
			GLuint count;
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bufId);
			counts = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
			count = counts[0];
			glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

			//glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &count);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
			GLenum err = glGetError();
			if (err > 0)
				cout << "Error AtomicBuffer.GetCounter():" << glewGetErrorString(err) << " " << err << endl;
			return count;
		}
		void ResetCounter(GLuint reset)
		{
			//GLuint *counts;
			while (glGetError());
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bufId);
			//counts = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
			//counts[0] = reset;
			//glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &reset); //reset counter to zero
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
			GLenum err = glGetError();
			if (err > 0)
				cout << "Error AtomicBuffer.ResetCounter():" << glewGetErrorString(err) << " " << err << endl;
		}
		~AtomicBuffer() { /*glDeleteBuffers(1, &bufId);*/ };
	};
}